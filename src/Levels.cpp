#include "Levels.hpp"
#include <algorithm>
#include <array>
#include <string_view>

namespace {
using Layout = std::array<std::string_view, 12>;
struct Stage {
    const char *name;
    const char *story;
    Layout tiles;
};
constexpr std::array<Stage, 10> Campaign{{
    {"Frozen Gate",
     "Seek the crown. Save the valley.",
     {{
         "....FFFFF....",
         "...FFFFFFF...",
         "..FFFFLFFFF..",
         "..FFFFFFFFF..",
         "..FFFFFFFFF..",
         "..RR.....RR..",
         "..RR.....RR..",
         "..RR.....RR..",
         "..RR.....RR..",
     }}},
    {"The Frostwood",
     "Find a path through the pines.",
     {{
         "......F......",
         ".....FFF.....",
         "....FFFFF....",
         "...FFFFFFF...",
         "..FFFFLFFFF..",
         "...FFFFFFF...",
         "..FFFFFFFFF..",
         ".FFFFFFFFFFF.",
         "...FFFFFFF...",
         "..FFFFFFFFF..",
         ".....RRR.....",
         ".....RRR.....",
     }}},
    {"Twin Peaks",
     "Climb beyond the twin peaks.",
     {{
         "...F.....F...",
         "..FFF...FFF..",
         ".FFFFF.FFFFF.",
         ".FFLFFFFFLFF.",
         ".FFFFFFFFFFF.",
         ".FFFFFFFFFFF.",
         ".FFFFFFFFFFF.",
         ".FFFFFFFFFFF.",
         ".RRFF...FFRR.",
         ".RR.......RR.",
     }}},
    {"Glacier Cave",
     "Venture beneath the glacier.",
     {{
         "..FFFFFFFFF..",
         ".FFFFFFFFFFF.",
         ".FFFFFFFFFFF.",
         ".RFFLFFFLFFR.",
         ".RRFFFFFFFRR.",
         ".RR.FF.FF.RR.",
         ".RR..F.F..RR.",
         ".RR.......RR.",
         ".RR.......RR.",
         ".RR.......RR.",
         ".RRFF...FFRR.",
         "..RR.....RR..",
     }}},
    {"Sky Bridge",
     "Cross the frozen river.",
     {{
         ".R.......R...",
         ".R.......R...",
         ".FFFFFFFFFFF.",
         ".FFFFFFFFFFF.",
         ".FFLFF...FRF.",
         ".FFFFF...FFF.",
         ".FFFFF...FFF.",
         ".RRRRR...RRR.",
         ".RRRRR...RRR.",
     }}},
    {"Old Watchtower",
     "Reach the old watchtower.",
     {{
         "..F.F.F.F.F..",
         "..FFFFFFFFF..",
         "..RRFFFFFRR..",
         "....FFFFF....",
         "....FFFFF....",
         "....FFLFF....",
         "....FFFFF....",
         "....FFFFF....",
         "....RFFFR....",
         "....RFFFR....",
         "...RR...RR...",
         "..RRR...RRR..",
     }}},
    {"Ancient Temple",
     "Unseal the ancient temple.",
     {{
         "......F......",
         ".....FFF.....",
         "....FFLFF....",
         "...FFFFFFF...",
         ".RRFFFFFFFRR.",
         "..RFFFLFFFR..",
         "..RFFFFFFFR..",
         "..RFFFFFFFR..",
         "..R.......R..",
         "..R.......R..",
         ".RRRR...RRRR.",
         ".FFFF...FFFF.",
     }}},
    {"Icebound Castle",
     "Breach the icebound castle.",
     {{
         ".F.F..F..F.F.",
         ".FFF.FFF.FFF.",
         ".FFLFFLFFLFF.",
         ".RFFFFFFFFFR.",
         ".FFFFFFFFFFF.",
         ".RFFFFFFFFFR.",
         ".RRFFFFFFFRR.",
         ".RRFF...FFRR.",
         ".RRFF...FFRR.",
         ".RRFF...FFRR.",
         ".RRRR...RRRR.",
         ".RRRR...RRRR.",
     }}},
    {"Frost Guardian",
     "Face the frost guardian.",
     {{
         "..RR.....RR..",
         ".RRFFFFFFFRR.",
         ".FFFFFFFFFFF.",
         ".F..FFFFF..F.",
         ".FFFLFFFLFFF.",
         "..FFFFFFFFF..",
         "...FFFFFFF...",
         "..RF.F.F.FR..",
         "..R.......R..",
         "...RRRRRRR...",
     }}},
    {"Frost Crown",
     "Reclaim the crown. Free the valley.",
     {{
         "..F...F...F..",
         "..FF.FFF.FF..",
         "..FFFFLFFFF..",
         ".FFFFFFFFFFF.",
         ".FFLFFFFFLFF.",
         ".RFFFFFFFFFR.",
         ".RRFFFFFFFRR.",
         "..RFFFFFFFR..",
         "..RRFFRFFRR..",
         "..RRFFRFFRR..",
         "....R...R....",
         "...RR...RR...",
     }}},
}};

const Stage &stageFor(int level) {
    return Campaign[std::clamp(level, 1, 10) - 1];
}
}

const char *Levels::name(int level) {
    return stageFor(level).name;
}
const char *Levels::story(int level) {
    return stageFor(level).story;
}

void Levels::generateLevel(int level, Bricks *BRICK[MAX_ROW][MAX_COL], int rowCount,
                           int columnCount, std::vector<sf::Vector2f> &pickupPositions) {
    pickupPositions.clear();
    rowCount = std::clamp(rowCount, 0, MAX_ROW);
    columnCount = std::clamp(columnCount, 0, MAX_COL);
    const auto &layout = stageFor(level).tiles;

    for (int r = 0; r < rowCount; ++r) {
        for (int c = 0; c < columnCount; ++c) {
            delete BRICK[r][c];
            BRICK[r][c] = nullptr;
            if (r >= static_cast<int>(layout.size()) || c >= static_cast<int>(layout[r].size()))
                continue;
            const char cell = layout[r][c];
            if (cell == 'F' || cell == 'R') {
                BRICK[r][c] = new Bricks((c + 1) * BRICK_WIDTH, (r + 4) * BRICK_HEIGHT,
                                         cell == 'R' ? BrickType::Rock : BrickType::Ice);
            }
        }
    }
    for (int r = 0; r < rowCount && r < static_cast<int>(layout.size()); ++r) {
        for (int c = 1; c + 1 < columnCount && c + 1 < static_cast<int>(layout[r].size()); ++c) {
            if (layout[r][c] != 'L')
                continue;
            const auto *left = BRICK[r][c - 1];
            const auto *right = BRICK[r][c + 1];
            if (!left || !right || left->type != BrickType::Ice || right->type != BrickType::Ice)
                continue;
            const auto bounds = left->brickSprite.getGlobalBounds();
            pickupPositions.push_back(
                {bounds.left + bounds.width / 2 + BRICK_WIDTH, bounds.top + bounds.height / 2});
        }
    }
}
