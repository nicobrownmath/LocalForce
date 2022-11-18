#include "LifeAPI.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <thread>
#include <mutex>
#include <stack>
#include <queue>
#include <chrono>
#include <map>
#include <set>

#define NUM_TRANSFORMS 16
#define NUM_SYMMETRIES 22

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
}
//from catforce
std::string GetRLE(const std::vector<std::vector<bool>> &life2d) {
  if (life2d.empty())
    return "";

  if (life2d[0].empty())
    return "";

  std::stringstream result;

  unsigned eol_count = 0;

  for (unsigned j = 0; j < life2d[0].size(); j++) {
    bool last_val = life2d[0][j];
    unsigned run_count = 0;

    for (const auto & i : life2d) {
      bool val = i[j];

      // Flush linefeeds if we find a live cell
      if (val && eol_count > 0) {
        if (eol_count > 1)
          result << eol_count;

        result << "$";

        eol_count = 0;
      }

      // Flush current run if val changes
      if (val == !last_val) {
        if (run_count > 1)
          result << run_count;

        if (last_val == 1)
          result << "o";
        else
          result << "b";

        run_count = 0;
      }

      run_count++;
      last_val = val;
    }

    // Flush run of live cells at end of line
    if (last_val) {
      if (run_count > 1)
        result << run_count;

      result << "o";

      run_count = 0;
    }

    eol_count++;
  }

  // Flush trailing linefeeds
  if (eol_count > 0) {
    if (eol_count > 1)
      result << eol_count;

    result << "$";

    eol_count = 0;
  }

  return result.str();
}
//from catforce
std::string GetRLE(const LifeState &s) {
  std::vector<std::vector<bool>> vec(N, std::vector<bool>(N));

  for (unsigned j = 0; j < N; j++)
    for (unsigned i = 0; i < N; i++)
      vec[i][j] = s.GetCell(i - 32, j - 32) == 1;

  return GetRLE(vec);
}
//also from catforce
//Note: Modified to be in the same order as the enum
std::vector<SymmetryTransform> SymmetryGroupFromEnum(const StaticSymmetry sym) {
  switch (sym) {
  case StaticSymmetry::C1:
    return {Identity};
  case StaticSymmetry::D2AcrossX:
    return {Identity, ReflectAcrossX};
  case StaticSymmetry::D2AcrossXEven:
    return {Identity, ReflectAcrossXEven};
  case StaticSymmetry::D2AcrossY:
    return {Identity, ReflectAcrossY};
  case StaticSymmetry::D2AcrossYEven:
    return {Identity, ReflectAcrossYEven};
  case StaticSymmetry::D2diagodd:
    return {Identity, ReflectAcrossYeqX};
  case StaticSymmetry::D2negdiagodd:
    return {Identity, ReflectAcrossYeqNegXP1};
  case StaticSymmetry::C2:
    return {Identity, Rotate180OddBoth};
  case StaticSymmetry::C2even:
    return {Identity, Rotate180EvenBoth};
  case StaticSymmetry::C2horizontaleven:
    return {Identity, Rotate180EvenHorizontal};
  case StaticSymmetry::C2verticaleven:
    return {Identity, Rotate180EvenVertical};
  case StaticSymmetry::C4:
    return {Identity, Rotate90, Rotate270, Rotate180OddBoth};
  case StaticSymmetry::C4even:
    return {Identity, Rotate90Even, Rotate270Even, Rotate180EvenBoth};
  case StaticSymmetry::D4:
    return {Identity, ReflectAcrossX, ReflectAcrossY, Rotate180OddBoth};
  case StaticSymmetry::D4even:
    return {Identity, ReflectAcrossXEven, ReflectAcrossYEven, Rotate180EvenBoth};
  case StaticSymmetry::D4horizontaleven:
    return {Identity, ReflectAcrossX, ReflectAcrossYEven, Rotate180EvenHorizontal};
  case StaticSymmetry::D4verticaleven:
    return {Identity, ReflectAcrossXEven, ReflectAcrossY, Rotate180EvenVertical};
  case StaticSymmetry::D4diag:
    return {Identity, Rotate180OddBoth, ReflectAcrossYeqX, ReflectAcrossYeqNegXP1};
  case StaticSymmetry::D4diageven:
    return {Identity, Rotate180EvenBoth, ReflectAcrossYeqX, ReflectAcrossYeqNegX};
  case StaticSymmetry::D8:
    return {Identity, ReflectAcrossX, ReflectAcrossY, Rotate90, Rotate270, Rotate180OddBoth, ReflectAcrossYeqX, ReflectAcrossYeqNegXP1};
  case StaticSymmetry::D8even:
    return {Identity, ReflectAcrossXEven, ReflectAcrossYEven, Rotate90Even, Rotate270Even, Rotate180EvenBoth, ReflectAcrossYeqX, ReflectAcrossYeqNegX};
  default: //D2negdiagevenSubgroupsOnly
    return {Identity, ReflectAcrossYeqNegX};
  }
}
StaticSymmetry SymmetryGroupToEnum(const std::vector<SymmetryTransform> &symmetryGroup) {
    switch (symmetryGroup.size()) {
        case 1: return C1;
        case 2: switch (symmetryGroup[1]) {
            case ReflectAcrossX: return D2AcrossX;
            case ReflectAcrossXEven: return D2AcrossXEven;
            case ReflectAcrossY: return D2AcrossY;
            case ReflectAcrossYEven: return D2AcrossYEven;
            case ReflectAcrossYeqX: return D2diagodd;
            case ReflectAcrossYeqNegXP1: return D2negdiagodd;
            case ReflectAcrossYeqNegX: return D2negdiagevenSubgroupsOnly;
            case Rotate180OddBoth: return C2;
            case Rotate180EvenBoth: return C2even;
            case Rotate180EvenHorizontal: return C2horizontaleven;
            case Rotate180EvenVertical: return C2verticaleven;
            default: {printf("error 1"); exit(1);}
        }
        case 4: switch (symmetryGroup[1]) {
            case Rotate90: return C4;
            case Rotate90Even: return C4even;
            case ReflectAcrossX: switch (symmetryGroup[2]) {
                case ReflectAcrossY: return D4;
                case ReflectAcrossYEven: return D4horizontaleven;
                default: {printf("error 2"); exit(1);}
            }
            case ReflectAcrossXEven: switch (symmetryGroup[2]) {
                case ReflectAcrossY: return D4verticaleven;
                case ReflectAcrossYEven: return D4even;
                default: {printf("error 3"); exit(1);}
            }
            case Rotate180OddBoth: return D4diag;
            case Rotate180EvenBoth: return D4diageven;
            default: {printf("error 4"); exit(1);}
        }
        case 8: switch (symmetryGroup[1]) {
            case ReflectAcrossX: return D8;
            case ReflectAcrossXEven: return D8even;
            default: {printf("error 5"); exit(1);}
        }
        //bizarre case
        default: return C1; break;
    }
}
std::vector<SymmetryTransform> SymmetryChainFromEnum(const StaticSymmetry sym) {
  switch (sym) {
  case StaticSymmetry::C1:
    return {};
  case StaticSymmetry::D2AcrossX:
    return {ReflectAcrossX};
  case StaticSymmetry::D2AcrossXEven:
    return {ReflectAcrossXEven};
  case StaticSymmetry::D2AcrossY:
    return {ReflectAcrossY};
  case StaticSymmetry::D2AcrossYEven:
    return {ReflectAcrossYEven};
  case StaticSymmetry::D2diagodd:
    return {ReflectAcrossYeqX};
  case StaticSymmetry::D2negdiagodd:
    return {ReflectAcrossYeqNegXP1};
  case StaticSymmetry::C2:
    return {Rotate180OddBoth};
  case StaticSymmetry::C2even:
    return {Rotate180EvenBoth};
  case StaticSymmetry::C2horizontaleven:
    return {Rotate180EvenHorizontal};
  case StaticSymmetry::C2verticaleven:
    return {Rotate180EvenVertical};
  case StaticSymmetry::C4:
    return {Rotate90, Rotate180OddBoth};
  case StaticSymmetry::C4even:
    return {Rotate90Even, Rotate180EvenBoth};
  case StaticSymmetry::D4:
    return {ReflectAcrossX, ReflectAcrossY};
  case StaticSymmetry::D4even:
    return {ReflectAcrossXEven, ReflectAcrossYEven};
  case StaticSymmetry::D4horizontaleven:
    return {ReflectAcrossYEven, ReflectAcrossX};
  case StaticSymmetry::D4verticaleven:
    return {ReflectAcrossXEven, ReflectAcrossY};
  case StaticSymmetry::D4diag:
    return {ReflectAcrossYeqX, ReflectAcrossYeqNegXP1};
  case StaticSymmetry::D4diageven:
    return {ReflectAcrossYeqX, ReflectAcrossYeqNegX};
  case StaticSymmetry::D8:
    return {ReflectAcrossX, ReflectAcrossY, Rotate90};
  case StaticSymmetry::D8even:
    return {ReflectAcrossXEven, ReflectAcrossYEven, Rotate90Even};
  default: //D2negdiagevenSubgroupsOnly
    return {ReflectAcrossYeqNegX};
  }
}
std::vector<SymmetryTransform> CharToTransforms(char ch) {
  switch (ch) {
  case '.':
    return SymmetryGroupFromEnum(StaticSymmetry::C1);
  case '|':
    return SymmetryGroupFromEnum(StaticSymmetry::D2AcrossY);
  case '-':
    return SymmetryGroupFromEnum(StaticSymmetry::D2AcrossX);
  case '\\':
    return SymmetryGroupFromEnum(StaticSymmetry::D2diagodd);
  case '/':
    return SymmetryGroupFromEnum(StaticSymmetry::D2negdiagodd);
  case '+':
  case '@':
    return SymmetryGroupFromEnum(StaticSymmetry::C4);
  case 'x':
    return {Identity, Rotate90, ReflectAcrossX, ReflectAcrossYeqX};
  case '*':
    return SymmetryGroupFromEnum(StaticSymmetry::D8);
  default:
    return SymmetryGroupFromEnum(StaticSymmetry::C1);
  }
}
StaticSymmetry GetPatternSymmetry(const LifeState &state, const std::vector<SymmetryTransform> &maxSymmetries) {
    std::vector<SymmetryTransform> symmetryGroup;
    for (auto &testTransform : maxSymmetries) {
        LifeState testState = state;
        testState.Transform(testTransform);
        if (state == testState) symmetryGroup.push_back(testTransform);
    }
    return SymmetryGroupToEnum(std::move(symmetryGroup));
}
StaticSymmetry SymmetryFromString(const std::string &name) {
  std::string start = name.substr(0, 2);
  std::string rest = name.substr(2);
  if (start == "D2") {
    if (rest == "-" or rest == "vertical") {
      return StaticSymmetry::D2AcrossX;
    } else if (rest == "-even" or rest == "verticaleven") {
      return StaticSymmetry::D2AcrossXEven;
    } else if (rest == "|" or rest == "horizontal") {
      return StaticSymmetry::D2AcrossY;
    } else if (rest == "|even" or rest == "horizontaleven") {
      return StaticSymmetry::D2AcrossYEven;
    } else if (rest == "/" or rest == "/odd") {
      return StaticSymmetry::D2negdiagodd;
    } else if (rest == "\\" or rest == "\\odd") {
      return StaticSymmetry::D2diagodd;
    }
  } else if (start == "C2") {
    if (rest == "" or rest == "_1") {
      return StaticSymmetry::C2;
    } else if (rest == "even" or rest == "_4") {
      return StaticSymmetry::C2even;
    } else if (rest == "horizontaleven" or rest == "|even") {
      return StaticSymmetry::C2horizontaleven;
    } else if (rest == "verticaleven" or rest == "-even" or rest == "_2") {
      return StaticSymmetry::C2verticaleven;
    }
  } else if (start == "C4") {
    if (rest == "" or rest == "_1") {
      return StaticSymmetry::C4;
    } else if (rest == "even" or rest == "_4") {
      return StaticSymmetry::C4even;
    }
  } else if (start == "D4") {
    std::string evenOddInfo = rest.substr(1);
    if (rest[0] == '+' or (rest.size() > 1 and rest[1] == '+')) {
      if (evenOddInfo == "" or rest == "_+1") {
        return StaticSymmetry::D4;
      } else if (evenOddInfo == "even" or rest == "_+4") {
        return StaticSymmetry::D4even;
      } else if (evenOddInfo == "verticaleven" or evenOddInfo == "-even" or
                 rest == "_+2") {
        return StaticSymmetry::D4verticaleven;
      } else if (evenOddInfo == "horizontaleven" or evenOddInfo == "|even") {
        return StaticSymmetry::D4horizontaleven;
      }
    } else if (rest[0] == 'x' or (rest.size() > 1 and rest[1] == 'x')) {
      if (evenOddInfo == "" or rest == "_x1") {
        return StaticSymmetry::D4diag;
      } else if (evenOddInfo == "even" or rest == "_x4") {
        return StaticSymmetry::D4diageven;
      }
    }
  } else if (start == "D8") {
    if (rest == "" or rest == "_1") {
      return StaticSymmetry::D8;
    } else if (rest == "even" or rest == "_4") {
      return StaticSymmetry::D8even;
    }
  }
  return StaticSymmetry::C1;
}
std::vector<StaticSymmetry> SymmetryPredecessorsOf(const StaticSymmetry sym) {
    switch (sym) {
        case D4: return {D2AcrossX, D2AcrossY};
        case D4verticaleven: return {D2AcrossXEven, D2AcrossY};
        case D4horizontaleven: return {D2AcrossX, D2AcrossYEven};
        case D4even: return {D2AcrossXEven, D2AcrossYEven};
        case D4diag:
        case D4diageven: return {D2diagodd, D2negdiagodd};
        case D8: return {D2AcrossX, D2AcrossY, D2diagodd, D2negdiagodd};
        case D8even: return {D2AcrossXEven, D2AcrossYEven, D2diagodd, D2negdiagodd};
        default: return {C1};
    }
}
std::vector<std::pair<StaticSymmetry, SymmetryTransform>> SymmetryImmediateSuccessorsOf(const StaticSymmetry sym) {
    switch (sym) {
        case C1: return {
            {C2, Rotate180OddBoth}, 
            {C2even, Rotate180EvenBoth}, 
            {C2verticaleven, Rotate180EvenVertical}, 
            {C2horizontaleven, Rotate180EvenHorizontal}, 
            {D2AcrossX, ReflectAcrossX},
            {D2AcrossXEven, ReflectAcrossXEven}, 
            {D2AcrossY, ReflectAcrossY}, 
            {D2AcrossYEven, ReflectAcrossYEven}, 
            {D2diagodd, ReflectAcrossYeqX}, 
            {D2negdiagodd, ReflectAcrossYeqNegXP1}, 
            {C4, Rotate90}, 
            {C4even, Rotate90Even}
        };
        case D2AcrossX: return {
            {D4, ReflectAcrossY}, 
            {D4horizontaleven, ReflectAcrossYEven}, 
            {D8, ReflectAcrossYeqX}
        };
        case D2AcrossXEven: return {
            {D4verticaleven, ReflectAcrossY}, 
            {D4even, ReflectAcrossYEven}, 
            {D8even, ReflectAcrossYeqX}
        };
        case D2AcrossY: return {
            {D4, ReflectAcrossX}, 
            {D4verticaleven, ReflectAcrossXEven}, 
            {D8, ReflectAcrossYeqX}
        };
        case D2AcrossYEven: return {
            {D4horizontaleven, ReflectAcrossX}, 
            {D4even, ReflectAcrossXEven}, 
            {D8even, ReflectAcrossYeqX}
        };
        case D2negdiagodd: return {
            {D4diag, ReflectAcrossYeqX}, //a bizarre case which gets converted to D4diageven
            {D8, ReflectAcrossX}, 
            {D8even, ReflectAcrossXEven}
        };
        case D2diagodd: return {
            {D4diag, ReflectAcrossYeqNegXP1}, //a bizarre case which gets converted to D4diageven
            //{D4diageven, ReflectAcrossYeqNegX},
            {D8, ReflectAcrossX}, 
            {D8even, ReflectAcrossXEven}
        };
        default: return {};
    }
}
const std::vector<SymmetryTransform> allTransforms = {
    Identity,
    ReflectAcrossXEven,
    ReflectAcrossX,
    ReflectAcrossYEven,
    ReflectAcrossY,
    Rotate90Even,
    Rotate90,
    Rotate270Even,
    Rotate270,
    Rotate180OddBoth,
    Rotate180EvenHorizontal,
    Rotate180EvenVertical,
    Rotate180EvenBoth,
    ReflectAcrossYeqX,
    ReflectAcrossYeqNegX,
    ReflectAcrossYeqNegXP1
};
std::pair<int, int> GetMaxUniqueMovementCoords(const SymmetryTransform &transform) {
    switch (transform) {
        case Identity: return {1, 1};
        case ReflectAcrossXEven:
        case ReflectAcrossX: return {1, 32};
        case ReflectAcrossYEven:
        case ReflectAcrossY: return {32, 1};
        case ReflectAcrossYeqX:
        case ReflectAcrossYeqNegX:
        case ReflectAcrossYeqNegXP1: return {64, 1};
        default: return {32, 32};
    }
}

class SearchParams {
    public:
    std::string pattern = "";
    bool patternRand = false;
    bool avoidRepeatPatternHistories = false;
    uint64_t seed = 0;
    int patternX = 0;
    int patternY = 0;

    unsigned threads = 1;
    unsigned maxOutputsPerRow = ~0U;
    unsigned minSaveInterval = 10;
    bool outputAll = false;
    bool ignoreElaboration = true;
    bool findPartials = true;
    unsigned partialCatsMustSurviveGens;
    bool findShuttles = true;
    std::string outputFile = "output-part.rle";
    std::string allOutputFile = "output-all.rle";
    std::string oscOutputFile = "output-osc.rle";

    unsigned minInteractionGeneration = 0;
    unsigned maxFirstInteractionGeneration = ~0U;
    unsigned maxFirstCatalystInteractionGeneration = ~0U;
    unsigned maxInteractionGeneration = ~0U;
    unsigned maxGenerationAfterLastInteraction = 1 << 31;
    unsigned maxGeneration = ~0U;

    unsigned minCatalysts = 0;
    unsigned maxCatalysts = ~0U;
    unsigned minCatalystsBeforeSymmetry = 0;
    unsigned maxCatalystsBeforeSymmetry = ~0U;
    unsigned minTransparentCatalysts = 0;
    unsigned maxTransparentCatalysts = ~0U;
    unsigned minSacrificialCatalysts = 0;
    unsigned maxSacrificialCatalysts = ~0U;
    unsigned maxSlots = ~0U;

    StaticSymmetry symmetry = C1;
    std::set<StaticSymmetry> possibleSymmetries = {C1};
    bool doDelayedSymmetryInteraction = true;
    bool allCatsCheckRecovery = false;
    bool checkRecoveryRequireUnique = false;
};

class CatContainerKey {
    public:
    LifeState state;
    LifeState matchState;
    int generation;

    bool operator==(const CatContainerKey &b) const {
        return state == b.state && matchState == b.matchState && generation == b.generation;
    }

    friend bool operator <(const CatContainerKey& a, const CatContainerKey& b) {
        if (a.state < b.state) return true;
        else if (b.state < a.state) return false;
        else if (a.matchState < b.matchState) return true;
        else if (b.matchState < a.matchState) return false;
        else return a.generation < b.generation;
    }

    CatContainerKey(const LifeState &state, const LifeState &matchState, const int &generation) {
        this->state = state;
        this->matchState = matchState;
        this->generation = generation;
    }
};

class CategoryContainer {
    public:
    std::map<CatContainerKey, std::vector<LifeState>> categories;
    std::vector<CatContainerKey> keys;

    bool hasBeenUpdated = false;
    unsigned size = 0;

    unsigned maxOutputsPerRow;

    bool Add(const LifeState &startState, const LifeState &finalState, const CatContainerKey &key) {
        if (categories.find(key) == categories.end()) keys.push_back(key);
        if (categories[key].size() < maxOutputsPerRow * 2)
        {
            categories[key].push_back(startState);
            categories[key].push_back(finalState);
            hasBeenUpdated = true;
            size++;
            return true;
        }
        return false;
    }

    std::string CategoriesRLE() {
        std::stringstream output;
        for (const CatContainerKey &key : keys) {
            std::vector<LifeState> results = categories[key];

            const unsigned Dist = 36 + 64;

            unsigned howmany = results.size();

            unsigned width = Dist * howmany;
            unsigned height = Dist;

            std::vector<std::vector<bool>> vec(width, std::vector<bool>(height));

            for (unsigned l = 0; l < howmany; l++)
                for (int j = 0; j < N; j++)
                    for (int i = 0; i < N; i++)
                    vec[Dist * l + i][j] = results[l].GetCell(i - 32, j - 32) == 1;

            output << GetRLE(vec);
            output << "36$";
        }

        return output.str();
    }
};

class Catalyst {
    public:
    bool firstUnderTransform[NUM_TRANSFORMS];
    LifeState fundamentalDomains[NUM_SYMMETRIES];
    LifeState symmetryExclusions[NUM_SYMMETRIES];

    LifeState state;
    LifeState locus;

    LifeState borderState;
    LifeState zoi;
    LifeState oneNeighbor;
    LifeState twoNeighbors;
    LifeState zoiR180;
    LifeState oneNeighborR180;
    LifeState twoNeighborsR180;
    LifeState locusZoiR180;
    LifeState locusOneNeighborR180;
    LifeState locusTwoNeighborsR180;

    unsigned recoveryTime;
    bool checkRecovery = false;
    bool checkRecoveryAlways = false;
    bool transparent = false;
    bool sacrificial = false;
    unsigned slots = 0;

    std::vector<std::pair<LifeState, LifeState>> forbidden;
    std::vector<LifeState> required;
    std::vector<LifeState> antirequired;
    std::vector<std::pair<LifeState, LifeState>> anyrequired; //consist of the required and antirequired parts

    static void AddCatalyst(std::vector<Catalyst> *catalysts, const std::vector<std::string> &elems) {
        //some of this comes from CatForce for the purposes of catalyst compatibility
        if (elems.size() <= 5) {
            std::cout << "Bad catalyst: Missing parameters" << std::endl;
            exit(0);
        }

        Catalyst catalyst;
        catalyst.state = LifeState::Parse(elems[1].c_str(), stoi(elems[3]), stoi(elems[4]));
        catalyst.locus = catalyst.state;

        catalyst.recoveryTime = stoi(elems[2]);

        std::vector<SymmetryTransform> transforms = CharToTransforms(elems[5].at(0));
        
        int index = 6;
        while (index < (int)elems.size()) {
            if (elems[index] == "forbidden") {
                LifeState forbiddenState = LifeState::Parse(elems[index + 1].c_str(), stoi(elems[index + 2]), stoi(elems[index + 3]));
                LifeState forbiddenStateBorder = forbiddenState.ZOI();
                forbiddenStateBorder.Copy(forbiddenState, ANDNOT);
                catalyst.forbidden.push_back({forbiddenState, forbiddenStateBorder});
                index += 4;
            }
            else if (elems[index] == "required") {
                catalyst.required.push_back(LifeState::Parse(elems[index + 1].c_str(), stoi(elems[index + 2]), stoi(elems[index + 3])));
                index += 4;
            }
            else if (elems[index] == "antirequired") {
                catalyst.antirequired.push_back(LifeState::Parse(elems[index + 1].c_str(), stoi(elems[index + 2]), stoi(elems[index + 3])));
                index += 4;
            }
            else if (elems[index] == "anyrequired") {
                catalyst.anyrequired.push_back({LifeState::Parse(elems[index + 1].c_str(), stoi(elems[index + 2]), stoi(elems[index + 3])), LifeState::Parse(elems[index + 4].c_str(), stoi(elems[index + 5]), stoi(elems[index + 6]))});
                index += 7;
            }
            else if (elems[index] == "locus") {
                catalyst.locus = LifeState::Parse(elems[index + 1].c_str(), stoi(elems[index + 2]), stoi(elems[index + 3]));
                index += 4;
            }
            else if (elems[index] == "check-recovery") {
                catalyst.checkRecovery = true;
                index++;
            }
            else if (elems[index] == "transparent") {
                catalyst.transparent = true;
                index++;
            }
            else if (elems[index] == "sacrificial") {
                catalyst.sacrificial = true;
                index++;
            }
            else if (elems[index] == "check-recovery-always") {
                catalyst.checkRecovery = true;
                catalyst.checkRecoveryAlways = true;
                index++;
            }
            else if (elems[index] == "slots") {
                catalyst.slots = stoi(elems[index + 1]);
                index += 2;
            }
            else {
                printf("Bad catalyst: Invalid parameter %s\n", elems[index].c_str());
                exit(0);
            }
        }

        int startingIndex = catalysts->size();
        for (auto &transform : transforms) {
            Catalyst transformedCatalyst = catalyst;
            transformedCatalyst.Transform(transform);
            transformedCatalyst.FillOutData();
            //fill out transforms
            for (int i = 0; i < NUM_TRANSFORMS; i++) {
                transformedCatalyst.firstUnderTransform[i] = true;
                for (int testCatIndex = startingIndex; testCatIndex < (int)catalysts->size(); testCatIndex++) {
                    LifeState earlierCatState = (*catalysts)[testCatIndex].state;
                    earlierCatState.Transform(static_cast<SymmetryTransform>(i));
                    if (!earlierCatState.FindMatches(transformedCatalyst.state).IsEmpty()) {
                        transformedCatalyst.firstUnderTransform[i] = false;
                        break;
                    }
                }
            }
            printf("Loaded catalyst %d\n", (int)catalysts->size());
            catalysts->push_back(std::move(transformedCatalyst));
        }
    }

    void Transform(SymmetryTransform transform) {
        state.Transform(transform);
        locus.Transform(transform);

        for (int i = 0; i < (int)forbidden.size(); i++) {
            forbidden[i].first.Transform(transform);
            forbidden[i].second.Transform(transform);
        }
        for (int i = 0; i < (int)required.size(); i++) {
            required[i].Transform(transform);
        }
        for (int i = 0; i < (int)antirequired.size(); i++) {
            antirequired[i].Transform(transform);
        }
        for (int i = 0; i < (int)anyrequired.size(); i++) {
            anyrequired[i].first.Transform(transform);
            anyrequired[i].second.Transform(transform);
        }
    }

    void FillOutData() {
        //all this stuff is calculated beforehand so we don't need to do it on the fly
        zoi = state.ZOI();
        borderState = zoi;
        borderState.Copy(state, ANDNOT);
        oneNeighbor = state.OneNeighbor();
        twoNeighbors = state.TwoNeighbors();

        zoiR180 = zoi;
        oneNeighborR180 = oneNeighbor;
        twoNeighborsR180 = twoNeighbors;
        zoiR180.Transform(Rotate180OddBoth);
        oneNeighborR180.Transform(Rotate180OddBoth);
        twoNeighborsR180.Transform(Rotate180OddBoth);

        locusZoiR180 = locus.ZOI();
        locusZoiR180.Transform(Rotate180OddBoth);
        locusOneNeighborR180 = oneNeighborR180;
        locusOneNeighborR180.Copy(locusZoiR180, AND);
        locusTwoNeighborsR180 = twoNeighborsR180;
        locusTwoNeighborsR180.Copy(locusZoiR180, AND);

        //fill out fundamental domains
        //registering these isn't super optimized but we only need to do it once
        for (unsigned i = 0; i < NUM_SYMMETRIES; i++) {
            fundamentalDomains[i] = LifeState();
            std::vector<SymmetryTransform> validTransforms;
            for (auto &testTransform : SymmetryGroupFromEnum(static_cast<StaticSymmetry>(i))) {
                if (testTransform == Identity) continue;
                LifeState transformedCat = state;
                transformedCat.Transform(testTransform);
                if (!transformedCat.FindMatches(state).IsEmpty()) {
                    validTransforms.push_back(testTransform);
                }
            }
            if (!validTransforms.empty()) {
                std::set<LifeState> accountedForStates;
                for (unsigned x = 0; x < 64; x++) {
                    for (unsigned y = 0; y < 64; y++) {
                        LifeState testPlacedState = state;
                        testPlacedState.Move(x, y);
                        if (accountedForStates.find(testPlacedState) == accountedForStates.end()) {
                            fundamentalDomains[i].Set(x, y);
                            for (auto &transform : validTransforms) {
                                LifeState addState = testPlacedState;
                                addState.Transform(transform);
                                accountedForStates.insert(addState);
                            }
                        }
                    }
                }
            }
            else {
                fundamentalDomains[i].Inverse();
            }
        }

        //fill out symmetry exclusions
        for (unsigned i = 0; i < NUM_SYMMETRIES; i++) {
            symmetryExclusions[i] = LifeState();
            for (int x = 0; x < 64; x++) {
                for (int y = 0; y < 64; y++) {
                    LifeState testPlacedState = state;
                    testPlacedState.Move(x, y);
                    LifeState testPlacedStateWithSymmetry = testPlacedState.GetSymChain(SymmetryChainFromEnum(static_cast<StaticSymmetry>(i)));
                    testPlacedStateWithSymmetry.Copy(testPlacedState, ANDNOT);
                    //check if the zoi from testPlacedState intersects the zoi from testPlacedStateWithSymmetry
                    //Note: This is very slightly overzealous: It doesn't allow catalysts which share a ZOI but are still stable
                    //The reason for this is that they would interfere with each other's neighbor counts
                    //TODO: However, it should be possible to allow (stable) configurations like this provided neither's locus is too close to the other
                    if (!testPlacedState.ZOI().AreDisjoint(testPlacedStateWithSymmetry.ZOI())) {
                        //invalid position
                        symmetryExclusions[i].Set(x, y);
                    }
                }
            }
        }
    }

    std::pair<bool, LifeState> CheckState(const LifeState &testState, const int &x, const int &y) {
        //pair is 'isValid, failurePoints'
        for (auto &requiredState : required) {
            LifeState missingCells = requiredState;
            missingCells.Move(x, y);
            missingCells.Copy(testState, ANDNOT);
            if (!missingCells.IsEmpty()) return {false, std::move(missingCells)};
        }
        for (auto &antirequiredState : antirequired) {
            LifeState presentState = antirequiredState;
            presentState.Move(x, y);
            presentState.Copy(testState, AND);
            if (!presentState.IsEmpty()) return {false, std::move(presentState)};
        }
        for (auto [forbiddenState, forbiddenStateBorder] : forbidden) {
            LifeState forbiddenError = forbiddenState;
            forbiddenError.Move(x, y);
            forbiddenError.Copy(testState, ANDNOT);
            LifeState forbiddenBorderError = forbiddenStateBorder;
            forbiddenBorderError.Move(x, y);
            forbiddenBorderError.Copy(testState, AND);
            if (forbiddenError.IsEmpty() && forbiddenBorderError.IsEmpty()) {
                LifeState outputState = forbiddenState.ZOI();
                forbiddenError.Move(x, y);
                return {false, std::move(outputState)};
            }
        }
        if (!anyrequired.empty())
        {
            bool anyRequirementMet = false;
            LifeState failureState;
            for (auto [requiredState, antirequiredState] : anyrequired) {
                LifeState errorState = requiredState;
                errorState.Move(x, y);
                errorState.Copy(testState, ANDNOT);
                LifeState antirequiredError = antirequiredState;
                antirequiredError.Move(x, y);
                antirequiredError.Copy(testState, AND);
                errorState.Join(std::move(antirequiredError));

                if (errorState.IsEmpty()) {
                    anyRequirementMet = true;
                    break;
                }
                else {
                    failureState.Join(std::move(errorState));
                }
            }
            if (!anyRequirementMet) return {false, std::move(failureState)};
        }
        LifeState catError = state;
        catError.Move(x, y);
        catError.Copy(testState, ANDNOT);
        LifeState catBorderError = borderState;
        catBorderError.Move(x, y);
        catBorderError.Copy(testState, AND);

        if (catError.IsEmpty() && catBorderError.IsEmpty()) return {true, LifeState()};

        LifeState outputState = zoi;
        outputState.Move(x, y);
        return {true, std::move(outputState)};
    }

    bool FirstUnderSymmetry(const std::vector<SymmetryTransform> transforms) {
        for (auto &transform : transforms) {
            if (!firstUnderTransform[static_cast<int>(transform)]) return false;
        }
        return true;
    }
};

class SearchData {
    public:
    unsigned generation;
    unsigned numCatalysts;
    unsigned numTransparentCatalysts;
    unsigned numSacrificialCatalysts;
    unsigned occupiedSlots;
    unsigned startCatIndex;
    unsigned startCatX;
    unsigned startCatY;
    unsigned lastCatGenOffset; //used for checkRecovery catalysts when placed in a filter
    unsigned transformLowerBound;
    unsigned maxCatalysts;
    bool lastAddedWasCatalyst;
    LifeState startState;
    LifeState matchState;
    LifeState currentState;
    LifeState past1Neighbor; //cells which have had exactly 1 neighbor in the pattern in some generation (catalysts impact by having 2 neighbors)
    LifeState past2Neighbors; //cells which have had exactly 2 neighbors in the pattern in some generation (catalysts impact by having 1 neighbor)
    LifeState past3Neighbors; //cells which have had exactly 3 neighbors in the pattern in some generation (catalysts impact by having any neighbors)

    std::vector<std::vector<unsigned>> catalysts; //values are of the form {index, x, y, timeSinceLastRestored}
    std::vector<std::tuple<LifeState, unsigned, unsigned, unsigned, unsigned, unsigned, unsigned>> filters;
    std::vector<std::tuple<unsigned, unsigned, unsigned, SymmetryTransform>> matchPoints;
    StaticSymmetry symmetry;
    StaticSymmetry maxSymmetry;
};

class LifeStateWithSymmetry {
    public:
    LifeState state;
    StaticSymmetry symmetry;

    bool operator==(const LifeStateWithSymmetry &b) const {
        return state == b.state && symmetry == b.symmetry;
    }

    friend bool operator <(const LifeStateWithSymmetry& a, const LifeStateWithSymmetry& b) {
        if (a.state < b.state) return true;
        else if (b.state < a.state) return false;
        else return static_cast<int>(a.symmetry) < static_cast<int>(b.symmetry);
    }

    LifeStateWithSymmetry(const LifeState &state, const StaticSymmetry &symmetry) {
        this->state = state;
        this->symmetry = symmetry;
    }
};

class LifeStateHistory {
    public:
    LifeState past1Neighbor;
    LifeState past2Neighbors;
    LifeState past3Neighbors;
    unsigned generation;

    bool operator==(const LifeStateHistory &b) const {
        return past1Neighbor == b.past1Neighbor && past2Neighbors == b.past2Neighbors && past3Neighbors == b.past3Neighbors && generation == b.generation;
    }

    friend bool operator <(const LifeStateHistory& a, const LifeStateHistory& b) {
        if (a.past1Neighbor < b.past1Neighbor) return true;
        else if (b.past1Neighbor < a.past1Neighbor) return false;
        else if (a.past2Neighbors < b.past2Neighbors) return true;
        else if (b.past2Neighbors < a.past2Neighbors) return false;
        else if (a.past3Neighbors < b.past3Neighbors) return true;
        else if (b.past3Neighbors < a.past3Neighbors) return false;
        else return a.generation < b.generation;
    }

    LifeStateHistory(const LifeState &past1Neighbor, const LifeState &past2Neighbors, const LifeState &past3Neighbors, const unsigned &generation) {
        this->past1Neighbor = past1Neighbor;
        this->past2Neighbors = past2Neighbors;
        this->past3Neighbors = past3Neighbors;
        this->generation = generation;
        
        //past 3 neighbor cells count as these for every sort of interaction
        this->past1Neighbor.Join(past3Neighbors);
        this->past2Neighbors.Join(past3Neighbors);
    }

    bool Contains(const LifeStateHistory &b) const {
        return generation >= b.generation && past1Neighbor.Contains(b.past1Neighbor) && past2Neighbors.Contains(b.past2Neighbors) && past3Neighbors.Contains(b.past3Neighbors);
    }
};

class Searcher {
    public:
    std::chrono::time_point<std::chrono::steady_clock> begin;
    std::chrono::time_point<std::chrono::steady_clock> lastSaveTime;
    std::chrono::time_point<std::chrono::steady_clock> lastSearchStartTime;
    double iterativeSearchTime = 0;
    double isolatedTime = 0;

    SearchParams params;
    std::vector<Catalyst> catalysts;

    CategoryContainer categoryContainer;
    CategoryContainer allOutputsCategoryContainer;
    CategoryContainer oscillatorCategoryContainer;
    std::mutex reportLock;
    std::mutex printLock;
    std::mutex categoryContainerLock;
    
    std::stack<SearchData> searchDataStack;
    std::mutex stackLock;

    std::vector<std::thread> threads;
    std::vector<bool> threadsActive;
    std::mutex threadActivityLock;

    int firstCatalystsLeft;
    int startingFirstCatalysts;

    uint64_t iterations = 0;

    std::map<LifeStateWithSymmetry, std::set<LifeStateHistory>> alreadyTestedLifeStates;
    std::mutex alreadyTestedLifeStatesLock;
    std::queue<LifeState> newSearchStatesQueue;
    std::set<LifeState> newSearchStatesSet;
    std::mutex newSearchStatesLock;

    std::set<LifeState> avoidOscs;
    std::mutex avoidOscContainerLock;

    //initialize search
    void Init(const std::string& fname) {
        //default to a random seed value if unspecified
        params.seed = ((std::chrono::duration<int64_t, std::nano>)std::chrono::steady_clock::now().time_since_epoch()).count();

        std::ifstream infile;
        infile.open(fname.c_str(), std::ifstream::in);
        if (!infile.good()) {
            std::cout << "Could not open file!" << std::endl;
            exit(1);
        }

        std::vector<std::vector<std::string>> catInputs;
        
        std::string line;
        while (std::getline(infile, line)) {
            std::vector<std::string> elems;
            split(line, ' ', elems);

            if (elems.size() < 2)
                continue;

            if (elems[0] == "pattern") {
                params.pattern = elems[1];

                if (elems.size() > 3) {
                    params.patternX = stoi(elems[2]);
                    params.patternY = stoi(elems[3]);
                }
            }
            if (elems[0] == "patternRand") {
                params.patternRand = true;
                params.patternX = stoi(elems[1]);
                params.patternY = stoi(elems[2]);
                if (elems.size() > 3) {
                    params.seed = stoull(elems[3]);
                }
            }
            if (elems[0] == "avoidRepeatPatternHistories") {
                params.avoidRepeatPatternHistories = (elems[1] == "true");
            }

            if (elems[0] == "symmetry") {
                params.symmetry = SymmetryFromString(elems[1]);
            }
            if (elems[0] == "targetSymmetries") {
                for (unsigned i = 1; i < elems.size(); i++) {
                    for (auto &transform : SymmetryPredecessorsOf(SymmetryFromString(elems[i]))) {
                        params.possibleSymmetries.insert(transform);
                    }
                    params.possibleSymmetries.insert(SymmetryFromString(elems[i]));
                }
            }
            if (elems[0] == "doDelayedSymmetryInteraction") {
                params.doDelayedSymmetryInteraction = (elems[1] == "true");
            }
            if (elems[0] == "allCatsCheckRecovery") {
                params.allCatsCheckRecovery = (elems[1] == "true");
            }
            if (elems[0] == "checkRecoveryRequireUnique") {
                params.checkRecoveryRequireUnique = (elems[1] == "true");
            }

            if (elems[0] == "threads") {
                params.threads = stoi(elems[1]);
            }
            if (elems[0] == "maxOutputsPerRow") {
                params.maxOutputsPerRow = stoi(elems[1]);
            }
            if (elems[0] == "minSaveInterval") {
                params.minSaveInterval = stoi(elems[1]);
            }

            if (elems[0] == "outputAll") {
                params.outputAll = (elems[1] == "true");
            }
            if (elems[0] == "ignoreElaboration") {
                params.ignoreElaboration = (elems[1] == "true");
            }
            if (elems[0] == "findShuttles") {
                params.findShuttles = (elems[1] == "true");
            }
            if (elems[0] == "findPartials") {
                params.findPartials = (elems[1] == "true");
                if (elems.size() > 2) {
                    params.partialCatsMustSurviveGens = stoi(elems[2]);
                }
            }
            if (elems[0] == "outputFile") {
                params.outputFile = elems[1] + "-part.rle";
                params.allOutputFile = elems[1] + "-all.rle";
                params.oscOutputFile = elems[1] + "-osc.rle";
            }

            if (elems[0] == "minInteractionGeneration") {
                params.minInteractionGeneration = stoi(elems[1]);
            }
            if (elems[0] == "maxFirstInteractionGeneration") {
                params.maxFirstInteractionGeneration = std::min((unsigned)stoi(elems[1]), params.maxFirstInteractionGeneration);
            }
            if (elems[0] == "maxFirstCatalystInteractionGeneration") {
                params.maxFirstInteractionGeneration = std::min((unsigned)stoi(elems[1]), params.maxFirstInteractionGeneration);
                params.maxFirstCatalystInteractionGeneration = std::min((unsigned)stoi(elems[1]), params.maxFirstCatalystInteractionGeneration);
            }
            if (elems[0] == "maxInteractionGeneration") {
                params.maxFirstInteractionGeneration = std::min((unsigned)stoi(elems[1]), params.maxFirstInteractionGeneration);
                params.maxFirstCatalystInteractionGeneration = std::min((unsigned)stoi(elems[1]), params.maxFirstCatalystInteractionGeneration);
                params.maxInteractionGeneration = std::min((unsigned)stoi(elems[1]), params.maxInteractionGeneration);
            }
            if (elems[0] == "maxGenerationAfterLastInteraction") {
                params.maxGenerationAfterLastInteraction = stoi(elems[1]);
            }
            if (elems[0] == "maxGeneration") {
                params.maxGeneration = stoi(elems[1]);
            }

            if (elems[0] == "minCatalysts") {
                params.minCatalysts = stoi(elems[1]);
            }
            if (elems[0] == "maxCatalysts") {
                params.maxCatalysts = stoi(elems[1]);
            }
            if (elems[0] == "minCatalystsBeforeSymmetry") {
                params.minCatalystsBeforeSymmetry = stoi(elems[1]);
            }
            if (elems[0] == "maxCatalystsBeforeSymmetry") {
                params.maxCatalystsBeforeSymmetry = stoi(elems[1]);
            }
            if (elems[0] == "minTransparentCatalysts") {
                params.minTransparentCatalysts = stoi(elems[1]);
            }
            if (elems[0] == "maxTransparentCatalysts") {
                params.maxTransparentCatalysts = stoi(elems[1]);
            }
            if (elems[0] == "minSacrificialCatalysts") {
                params.minSacrificialCatalysts = stoi(elems[1]);
            }
            if (elems[0] == "maxSacrificialCatalysts") {
                params.maxSacrificialCatalysts = stoi(elems[1]);
            }
            if (elems[0] == "maxSlots") {
                params.maxSlots = stoi(elems[1]);
            }

            if (elems[0] == "cat")
            {
                Catalyst::AddCatalyst(&catalysts, elems);
            }
        }

        infile.close();

        //option for all cats checking recovery
        //speeds up search a lot but misses some things
        if (params.allCatsCheckRecovery) {
            for (auto &catalyst : catalysts) {
                catalyst.checkRecovery = true;
            }
        }

        categoryContainer.maxOutputsPerRow = params.maxOutputsPerRow;
        allOutputsCategoryContainer.maxOutputsPerRow = params.maxOutputsPerRow;
        oscillatorCategoryContainer.maxOutputsPerRow = params.maxOutputsPerRow;
    }

    void Run() {
        if (params.patternRand) {
            printf("Running search with seed %llu:\n\n", params.seed);
            PRNG::seed(params.seed);
        }

        do {
            lastSearchStartTime = std::chrono::steady_clock::now();
            IterativeSearch(NextSearchData());
            firstCatalystsLeft = searchDataStack.size();
            startingFirstCatalysts = firstCatalystsLeft;
            printf("Starting first iterations: %d\n", firstCatalystsLeft);

            //initialize threads
            threadsActive = std::vector<bool>();
            threads = std::vector<std::thread>();
            for (unsigned i = 0; i < params.threads; i++) {
                threadsActive.push_back(true);
                threads.push_back(std::thread(&Searcher::SearchThroughStack, this, i));
            }

            //wait for threads to finish
            for (unsigned i = 0; i < params.threads; i++)
            {
                threads[i].join();
            }

            Report(); //final report
            if (params.patternRand) printf("\n\n\nInitializing next search\n");
        } while (params.patternRand);
    }

    SearchData NextSearchData() {
        LifeState state;
        if (params.patternRand) {
            //TODO: Move to symmetric position if possible
            if (newSearchStatesQueue.empty()) {
                state = GetNewUniqueStateWithSymmetry(params.symmetry);
                newSearchStatesSet.insert(std::get<0>(state.StandardizedWithTransforms(C1)));
            } else {
                state = std::move(newSearchStatesQueue.front());
                newSearchStatesQueue.pop();
            }
        }
        else {
            state = LifeState::Parse(params.pattern.c_str(), params.patternX, params.patternY);
            state.SymChain(SymmetryChainFromEnum(params.symmetry));
        }
        return InitialSearchData(std::move(state));
    }

    SearchData InitialSearchData(const LifeState &state) {
        SearchData searchData;
        searchData.maxCatalysts = params.maxCatalysts;
        searchData.generation = 0;
        searchData.numCatalysts = 0;
        searchData.numTransparentCatalysts = 0;
        searchData.numSacrificialCatalysts = 0;
        searchData.occupiedSlots = 0;
        searchData.startCatIndex = 0;
        searchData.startCatX = 0;
        searchData.startCatY = 0;
        searchData.lastCatGenOffset = 0;
        searchData.lastAddedWasCatalyst = false;
        searchData.symmetry = params.symmetry;
        searchData.startState = state;
        searchData.matchState = searchData.startState;
        searchData.currentState = searchData.startState;
        searchData.past1Neighbor = LifeState();
        searchData.past2Neighbors = LifeState();
        searchData.past3Neighbors = searchData.startState;
        searchData.catalysts = {};
        searchData.filters = {};
        searchData.matchPoints = {};
        searchData.maxSymmetry = GetPatternSymmetry(searchData.startState, allTransforms);
        return searchData;
    }

    LifeState GetNewUniqueStateWithSymmetry(StaticSymmetry symmetry) {
        while (true) {
            //TODO: Instead of continuing to choose at random, advance lexicographically until we either loop or return, at which point give up and quit out
            LifeState output = LifeState::RandomState();
            output.Copy(LifeState::SolidRect(-(params.patternX / 2), -(params.patternY / 2), params.patternX, params.patternY), AND);
            output.SymChain(SymmetryChainFromEnum(symmetry));
            if (newSearchStatesSet.find(std::get<0>(output.StandardizedWithTransforms(C1))) == newSearchStatesSet.end())
                return std::get<0>(output.StandardizedWithTransforms(C1));
        }
    }

    void SearchThroughStack(const int &index) {
        SearchData searchData;
        bool allThreadsInactive = false;
        while (!allThreadsInactive) {
            bool doSearch = false;
            stackLock.lock();
            if (!searchDataStack.empty()) {
                searchData = std::move(searchDataStack.top());
                searchDataStack.pop();
                if (firstCatalystsLeft > (int)searchDataStack.size()) firstCatalystsLeft = searchDataStack.size();
                doSearch = true;
            }
            stackLock.unlock();
            if (doSearch) {
                threadActivityLock.lock();
                threadsActive[index] = true;
                threadActivityLock.unlock();
                IterativeSearch(std::move(searchData));
            }
            else {
                allThreadsInactive = true;
                threadActivityLock.lock();
                threadsActive[index] = false;
                for (unsigned i = 0; i < params.threads; i++) {
                    if (threadsActive[i]) {
                        allThreadsInactive = false;
                        break;
                    }
                }
                threadActivityLock.unlock();
                //if all threads are done, exit all
                if (allThreadsInactive) {
                    return;
                }
            }

            //try reporting
            std::chrono::time_point<std::chrono::steady_clock> timePoint = std::chrono::steady_clock::now();
            reportLock.lock();
            std::chrono::duration<double> currentTime = timePoint - lastSaveTime;
            if (currentTime.count() > params.minSaveInterval) { //only save reports after intervals because it's expensive and doesn't thread well
                lastSaveTime = timePoint;
                Report();
            }
            reportLock.unlock();
        }
    }

    void IterativeSearch(const SearchData &searchData) {
        std::chrono::time_point<std::chrono::steady_clock> methodStartTime = std::chrono::steady_clock::now();
        iterations++;

        if ((params.avoidRepeatPatternHistories || searchData.generation == 0 || searchData.symmetry == params.symmetry) && params.patternRand && searchData.numCatalysts == 0) {
            //add pattern-symmetry-history to the already tested
            //if it's contained in prior tests skip it
            std::tuple<LifeState, int, int, SymmetryTransform> standardizedData = searchData.currentState.StandardizedWithTransforms(searchData.symmetry);
            
            LifeState transformedPast1Neighbor = searchData.past1Neighbor;
            transformedPast1Neighbor.Transform(std::get<3>(standardizedData));
            transformedPast1Neighbor.Move(std::get<1>(standardizedData), std::get<2>(standardizedData));
            LifeState transformedPast2Neighbors = searchData.past2Neighbors;
            transformedPast2Neighbors.Transform(std::get<3>(standardizedData));
            transformedPast2Neighbors.Move(std::get<1>(standardizedData), std::get<2>(standardizedData));
            LifeState transformedPast3Neighbors = searchData.past3Neighbors;
            transformedPast3Neighbors.Transform(std::get<3>(standardizedData));
            transformedPast3Neighbors.Move(std::get<1>(standardizedData), std::get<2>(standardizedData));
            
            LifeStateHistory newHistory = LifeStateHistory(transformedPast1Neighbor, transformedPast2Neighbors, transformedPast3Neighbors, searchData.generation);
            LifeStateWithSymmetry stateWithSymmetry = LifeStateWithSymmetry(std::get<0>(standardizedData), searchData.symmetry);
            alreadyTestedLifeStatesLock.lock();
            if (alreadyTestedLifeStates.find(stateWithSymmetry) != alreadyTestedLifeStates.end()) {
                //if a history in here contains our own, remove it
                //if a history is contained by our own, it's already been searched so break
                //note: for purposes of containment, 3 neighbor cells contain both 1 and 2 neighbor cells
                std::vector<LifeStateHistory> obsolete;
                for (auto &history : alreadyTestedLifeStates[stateWithSymmetry]) {
                    if (newHistory.Contains(history)) {
                        alreadyTestedLifeStatesLock.unlock();
                        iterativeSearchTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - methodStartTime)).count();
                        return;
                    }
                    else if (history.Contains(newHistory)) {
                        obsolete.push_back(history);
                    }
                }
                for (auto &obsoleteHistory : obsolete) {
                    alreadyTestedLifeStates[stateWithSymmetry].erase(obsoleteHistory);
                }
            }
            else {
                alreadyTestedLifeStates[stateWithSymmetry] = {};
            }
            alreadyTestedLifeStates[stateWithSymmetry].insert(newHistory);
            alreadyTestedLifeStatesLock.unlock();
        }
        if (params.patternRand && searchData.symmetry == C1 && searchData.numCatalysts == 0) {
            printLock.lock();
            printf("Searching state:\n");
            //TODO: print in just a rectangle
            searchData.currentState.Print();
            printLock.unlock();
        }

        std::vector<LifeState> stateEvolution = {searchData.currentState};
        std::vector<LifeState> past1NeighborEvolution = {searchData.past1Neighbor};
        std::vector<LifeState> past2NeighborsEvolution = {searchData.past2Neighbors};
        std::vector<LifeState> past3NeighborsEvolution = {searchData.past3Neighbors};
        std::vector<std::vector<std::vector<unsigned>>> catalystsEvolution = {searchData.catalysts};

        bool isValid = true;

        unsigned eventualPeriod = 0;
        int minValidPeriod = 0;
        for (auto &catData : searchData.catalysts) {
            if ((int)catalysts[catData[0]].recoveryTime > minValidPeriod && !catalysts[catData[0]].sacrificial) minValidPeriod = catalysts[catData[0]].recoveryTime;
        }

        LifeState filterState = LifeState();

        std::vector<std::tuple<unsigned, unsigned, unsigned, SymmetryTransform>> newMatchPoints;

        std::vector<std::pair<StaticSymmetry, SymmetryTransform>> availableSymmetries = GetAvailableSymmetries(searchData.symmetry);
        bool searchSymmetries = !availableSymmetries.empty();

        unsigned maxGenerationAllowed = std::min(params.maxGeneration, params.maxGenerationAfterLastInteraction + searchData.generation);

        //int dontCheckTransparentLastCatalyst = searchData.lastAddedWasCatalyst ? 1 : 0;
        //bool bonusCatalystFromTransparentWorking = false;

        for (unsigned generation = searchData.generation; generation < maxGenerationAllowed; generation++) {
            int generationIndex = generation - searchData.generation;

            stateEvolution.push_back(stateEvolution[generationIndex]);
            stateEvolution[generationIndex + 1].Step();

            //check catalyst validity
            catalystsEvolution.push_back(catalystsEvolution[generationIndex]);
            for (int catID = searchData.catalysts.size() - 1; catID >= 0; catID--) {
                //these check in reverse order so that recently-added checkRecovery catalysts are evaluated first
                std::pair<bool, LifeState> filtering = catalysts[catalystsEvolution[generationIndex][catID][0]].CheckState(stateEvolution[generationIndex + 1], catalystsEvolution[generationIndex][catID][1], catalystsEvolution[generationIndex][catID][2]);
                if (!filtering.first) {
                    //catalyst broken
                    isValid = false;
                    //if we have the checkRecovery flag and this is the newest catalyst and generation count is low enough, totally invalid so return
                    if (catalysts[catalystsEvolution[generationIndex][catID][0]].checkRecoveryAlways /*|| (catID < (int)catalystsEvolution[generationIndex].size() - dontCheckTransparentLastCatalyst)*/) {
                        iterativeSearchTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - methodStartTime)).count();
                        return;
                    }
                    filterState = std::move(filtering.second);
                    break;
                }
                else if (!filtering.second.IsEmpty() && !catalysts[searchData.catalysts[catID][0]].sacrificial) {
                    //catalyst active but not restored
                    catalystsEvolution[generationIndex + 1][catID][3] = catalystsEvolution[generationIndex][catID][3] + 1;
                    if (catalystsEvolution[generationIndex + 1][catID][3] > catalysts[catalystsEvolution[generationIndex][catID][0]].recoveryTime) {
                        //if we have the checkRecovery flag and this is the newest catalyst and generation count is low enough, totally invalid so return
                        if (catalysts[catalystsEvolution[generationIndex][catID][0]].checkRecoveryAlways /*|| (catID < (int)catalystsEvolution[generationIndex].size() - dontCheckTransparentLastCatalyst)*/) {
                            iterativeSearchTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - methodStartTime)).count();
                            return;
                        }
                        isValid = false;
                        filterState = std::move(filtering.second);
                        break;
                    }
                }
                else {
                    //catalyst restored
                    catalystsEvolution[generationIndex + 1][catID][3] = 0;

                    //TODO: This may sometimes be checking too early?
                    /*if (catID >= (int)catalystsEvolution[generationIndex].size() - dontCheckTransparentLastCatalyst && !catalysts[catalystsEvolution[generationIndex][catID][0]].sacrificial) {
                        bonusCatalystFromTransparentWorking = true;
                    }*/
                }
            }
            if (!isValid) break;

            if (generation <= params.maxInteractionGeneration && (searchData.numCatalysts < searchData.maxCatalysts || (searchSymmetries && (generation == 0 || params.doDelayedSymmetryInteraction)))) { //these are only needed for catalyst placement
                past1NeighborEvolution.push_back(past1NeighborEvolution[generationIndex]);
                past2NeighborsEvolution.push_back(past2NeighborsEvolution[generationIndex]);
                past3NeighborsEvolution.push_back(past3NeighborsEvolution[generationIndex]);

                past1NeighborEvolution[generationIndex + 1].Join(stateEvolution[generationIndex].OneNeighbor());
                past2NeighborsEvolution[generationIndex + 1].Join(stateEvolution[generationIndex].TwoNeighbors());
                past3NeighborsEvolution[generationIndex + 1].Join(stateEvolution[generationIndex].ThreeNeighbors());

                //if patternRand and numCatalysts = 0, add this to the state's possible histories
                if (params.avoidRepeatPatternHistories && params.patternRand && (searchData.numCatalysts == 0 && generation < params.maxFirstCatalystInteractionGeneration)) {
                    std::tuple<LifeState, int, int, SymmetryTransform> standardizedData = stateEvolution[generationIndex + 1].StandardizedWithTransforms(searchData.symmetry);
                    
                    LifeState transformedPast1Neighbor = past1NeighborEvolution[generationIndex + 1];
                    transformedPast1Neighbor.Transform(std::get<3>(standardizedData));
                    transformedPast1Neighbor.Move(std::get<1>(standardizedData), std::get<2>(standardizedData));
                    LifeState transformedPast2Neighbors = past2NeighborsEvolution[generationIndex + 1];
                    transformedPast2Neighbors.Transform(std::get<3>(standardizedData));
                    transformedPast2Neighbors.Move(std::get<1>(standardizedData), std::get<2>(standardizedData));
                    LifeState transformedPast3Neighbors = past3NeighborsEvolution[generationIndex + 1];
                    transformedPast3Neighbors.Transform(std::get<3>(standardizedData));
                    transformedPast3Neighbors.Move(std::get<1>(standardizedData), std::get<2>(standardizedData));
            
                    LifeStateHistory newHistory = LifeStateHistory(transformedPast1Neighbor, transformedPast2Neighbors, transformedPast3Neighbors, generationIndex + 1 + searchData.generation);
                    LifeStateWithSymmetry stateWithSymmetry = LifeStateWithSymmetry(std::get<0>(standardizedData), searchData.symmetry);
                    bool historyAlreadyCovered = false;
                    alreadyTestedLifeStatesLock.lock();
                    if (alreadyTestedLifeStates.find(stateWithSymmetry) != alreadyTestedLifeStates.end()) {
                        //if a history in here contains our own, remove it
                        //if a history is contained by our own, it's already been searched so break
                        //note: for purposes of containment, 3 neighbor cells contain both 1 and 2 neighbor cells
                        std::vector<LifeStateHistory> obsolete;
                        for (auto &history : alreadyTestedLifeStates[stateWithSymmetry]) {
                            if (newHistory.Contains(history)) {
                                historyAlreadyCovered = true;
                                break;
                            }
                            else if (history.Contains(newHistory)) {
                                obsolete.push_back(history);
                            }
                        }
                        if (!historyAlreadyCovered)
                            for (auto &obsoleteHistory : obsolete) {
                                alreadyTestedLifeStates[stateWithSymmetry].erase(obsoleteHistory);
                            }
                    }
                    else {
                        alreadyTestedLifeStates[stateWithSymmetry] = {};
                    }
                    if (!historyAlreadyCovered) {
                        alreadyTestedLifeStates[stateWithSymmetry].insert(newHistory);
                    }
                    //TODO: This can be a pretty massive memory drain! I should try to minimize additions to it
                    //  Possibly only add first-generation things?
                    alreadyTestedLifeStatesLock.unlock();
                }
            }

            //isolate interesting results
            //only run this if we've interacted with the most recent catalyst
            if ((unsigned)generationIndex > searchData.lastCatGenOffset && params.findPartials) {
                //TODO: Don't run some of these due to redundancy if matchPoints has certain symmetries
                //TODO: Maybe also require catalyst union under transform to be stable?
                LifeState matchStateBorder = searchData.matchState.ZOI();
                matchStateBorder.Copy(searchData.matchState, ANDNOT);
                LifeState catalystsState = searchData.startState;
                catalystsState.Copy(searchData.matchState, ANDNOT);
                std::vector<std::tuple<unsigned, unsigned, unsigned, SymmetryTransform>> oldMatchPoints = params.ignoreElaboration ? searchData.matchPoints : std::vector<std::tuple<unsigned, unsigned, unsigned, SymmetryTransform>>();
                CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, Identity);
                if (params.findShuttles) {
                    switch (searchData.symmetry) {
                        case C1:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 64, ReflectAcrossX);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 1, ReflectAcrossY);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 1, ReflectAcrossYeqX, -1);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 1, ReflectAcrossYeqNegXP1, 1);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 64, Rotate90);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 64, Rotate180OddBoth);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 64, Rotate270);
                            break;
                        case D2AcrossX:
                        case D2AcrossXEven:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 1, ReflectAcrossY);
                            break;
                        case D2AcrossY:
                        case D2AcrossYEven:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 64, ReflectAcrossX);
                            break;
                        case D2diagodd:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 1, ReflectAcrossYeqNegXP1, 1);
                            break;
                        case D2negdiagodd:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 64, 1, ReflectAcrossYeqX, -1);
                            break;
                        case C2:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, Rotate90);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, ReflectAcrossX);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, ReflectAcrossYeqX);
                            break;
                        case C2even:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, Rotate90Even);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, ReflectAcrossXEven);
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, ReflectAcrossYeqX);
                            break;
                        case C2verticaleven:
                        case C4even:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, ReflectAcrossXEven);
                            break;
                        case C2horizontaleven:
                        case C4:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, ReflectAcrossX);
                            break;
                        case D4:
                        case D4diag:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, Rotate90);
                            break;
                        case D4even:
                        case D4diageven:
                            CheckMatches(&newMatchPoints, oldMatchPoints, stateEvolution[generationIndex + 1], searchData.matchState, matchStateBorder, catalystsState, generationIndex + 1 + searchData.generation, 1, 1, Rotate90Even);
                            break;
                        default: break;
                    }
                }
            }

            //stop with all the evolving if we're periodic
            std::vector<bool> catsRestored;
            for (unsigned i = 0; i < (unsigned)searchData.catalysts.size(); i++) {
                catsRestored.push_back(false);
            }
            for (int testGeneration = generationIndex; testGeneration >= 0; testGeneration--) {
                //periodicity check
                bool allCatsRestored = true;
                for (unsigned i = 0; i < (unsigned)searchData.catalysts.size(); i++) {
                    if (catalystsEvolution[testGeneration][i][3] == 0) catsRestored[i] = true;
                    if (!catsRestored[i]) allCatsRestored = false;
                }
                if (stateEvolution[generationIndex + 1] == stateEvolution[testGeneration]) {
                    if (allCatsRestored) {
                        //catalysts are restored
                        eventualPeriod = generationIndex + 1 - testGeneration;
                    } else {
                        //catalysts aren't restored within our period, so they'll never be restored
                        if (searchData.numCatalysts == searchData.maxCatalysts && !searchSymmetries) isValid = false;
                        //only need to bother if there are few enough catalysts/symmetries
                        else {
                            //otherwise, we're not invalid yet but will be
                            //skip to the actual point of failure here for our filters (particularly important for transparent cats or others with long recovery times)
                            for (int catID = searchData.catalysts.size() - 1; catID >= 0; catID--) {
                                //these check in reverse order so that recently-added checkRecovery catalysts are evaluated first
                                if (!catsRestored[catID] && !catalysts[searchData.catalysts[catID][0]].sacrificial) {
                                    //checkRecovery catalyst failure
                                    if (catalysts[searchData.catalysts[catID][0]].checkRecoveryAlways /*|| (catID < (int)searchData.catalysts.size() - dontCheckTransparentLastCatalyst)*/) {
                                        iterativeSearchTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - methodStartTime)).count();
                                        return;
                                    }
                                    
                                    //TODO: This currently fails on cases where the catalyst would only fail after params.maxGeneration, it probably shouldn't
                                    int gensUntilCatFailure = catalysts[searchData.catalysts[catID][0]].recoveryTime + 1 - catalystsEvolution[testGeneration][catID][3];
                                    int modelGeneration = testGeneration + gensUntilCatFailure % (generationIndex + 1 - testGeneration);
                                    //This could maybe be cached but I'm not sure that'd actually be faster
                                    filterState = catalysts[searchData.catalysts[catID][0]].CheckState(stateEvolution[modelGeneration], searchData.catalysts[catID][1], searchData.catalysts[catID][2]).second;
                                    filterState.Inflate(testGeneration + gensUntilCatFailure - (generationIndex + 1));
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
            }
            if (!isValid) break;
            if (eventualPeriod > 0) break;

            //if we're a symmetric version of something contained in the initial start region, run a new search with that and break
            if (params.patternRand && searchData.numCatalysts == 0 && (params.doDelayedSymmetryInteraction || searchData.symmetry == C1)) {
                LifeState mask1 = LifeState::SolidRect(-params.patternX / 2, -params.patternY / 2, params.patternX, params.patternY);
                LifeState mask2 = LifeState::SolidRect(-params.patternY / 2, -params.patternX / 2, params.patternY, params.patternX);
                bool foundDecomposition = false;
                //TODO: This part might be slow, so I should a. diagnostic it and b. try to call it less
                for (int x = 0; x < 64; x++) {
                    for (int y = 0; y < 64; y++) {
                        bool mask1Works = mask1.GetSymChain(x, y, SymmetryChainFromEnum(searchData.symmetry)).Contains(stateEvolution[generationIndex + 1]);
                        bool mask2Works = mask2.GetSymChain(x, y, SymmetryChainFromEnum(searchData.symmetry)).Contains(stateEvolution[generationIndex + 1]);
                        if (mask1Works || mask2Works) {
                            //run this soon, we want to be biased a bit towards more common patterns
                            //the exact criterion we use is a bit pointless though it should at least accept everything C1
                            //TODO: This is actually very weird prioritization
                            if (searchData.symmetry == C1 || true) {
                                LifeState newStartState = stateEvolution[generationIndex + 1];
                                newStartState.JoinAND(mask1Works ? mask1 : mask2, x, y);

                                bool success = false;
                                newSearchStatesLock.lock();
                                if (newSearchStatesSet.find(std::get<0>(newStartState.StandardizedWithTransforms(C1))) == newSearchStatesSet.end()) {
                                    newSearchStatesSet.insert(std::get<0>(newStartState.StandardizedWithTransforms(C1)));
                                    success = true;
                                }
                                newSearchStatesLock.unlock();

                                if (success) {
                                    newStartState.Move(-x, -y);
                                    //do these immediately so we don't delay common patterns forever (this doesn't really matter tbh)
                                    if (searchData.symmetry == C1) IterativeSearch(InitialSearchData(std::move(newStartState)));
                                    else {
                                        newSearchStatesLock.lock();
                                        //TODO: Ensure we don't also run same-gen cat/sym checks since those would duplicate
                                        newSearchStatesQueue.push(std::move(newStartState));
                                        newSearchStatesLock.unlock();
                                    }
                                }
                            }

                            foundDecomposition = true;
                            break;
                        }
                    }
                    if (foundDecomposition) break;
                }
                if (foundDecomposition) break;
            }
        }
        //TODO: It should be possible to lump all these checks together to minimize the amount of steps
        for (int catID = 0; catID < (int)searchData.catalysts.size(); catID++) {
            if (catalysts[searchData.catalysts[catID][0]].checkRecoveryAlways) {
                LifeState testState = stateEvolution[stateEvolution.size() - 1];
                int remainingSteps = (int)catalysts[searchData.catalysts[catID][0]].recoveryTime - (int)catalystsEvolution[stateEvolution.size() - 1][catID][3];
                bool recovered = false;
                for (unsigned i = 0; (int)i < remainingSteps; i++) {
                    testState.Step();
                    std::pair<bool, LifeState> filtering = catalysts[searchData.catalysts[catID][0]].CheckState(testState, searchData.catalysts[catID][1], searchData.catalysts[catID][2]);
                    if (!filtering.first) {
                        break;
                    }
                    else if (filtering.second.IsEmpty()) {
                        recovered = true;
                        break;
                    }
                }
                if (!recovered) {
                    iterativeSearchTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - methodStartTime)).count();
                    return;
                }
            }
        }

        //TODO: Make this more adjustable
        //TODO: Also need to test with other checkrecoveryalways catalysts?
        //TODO: Option to grant extra catalysts if we're interesting?
        //TODO: Only invalidate if fails up to a certain no. of gens after last cat interaction?
        /*for (int catID = 0; catID < (int)searchData.catalysts.size(); catID++) {
            if (!catalysts[searchData.catalysts[catID][0]].sacrificial) {
                LifeState testState = stateEvolution[stateEvolution.size() - 1];
                int remainingSteps = (int)catalysts[searchData.catalysts[catID][0]].recoveryTime - (int)catalystsEvolution[stateEvolution.size() - 1][catID][3];
                bool recovered = false;
                for (unsigned i = 0; (int)i < remainingSteps; i++) {
                    testState.Step();
                    std::pair<bool, LifeState> filtering = catalysts[searchData.catalysts[catID][0]].CheckState(testState, searchData.catalysts[catID][1], searchData.catalysts[catID][2]);
                    if (!filtering.first) {
                        break;
                    }
                    else if (filtering.second.IsEmpty()) {
                        recovered = true;
                        break;
                    }
                }
                if (!recovered) {
                    if (catID < (int)searchData.catalysts.size() - dontCheckTransparentLastCatalyst) {
                        iterativeSearchTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - methodStartTime)).count();
                        return;
                    }
                } else {
                    if (catID >= (int)searchData.catalysts.size() - dontCheckTransparentLastCatalyst) {
                        bonusCatalystFromTransparentWorking = true;
                        break;
                    }
                }
            }
        }*/
        //oh no
        //TODO: Make it so that each catalyst can only grant the bonus once
        /*if ((bonusCatalystFromTransparentWorking || (params.findPartials && !newMatchPoints.empty() && (searchData.numCatalysts > 0 || searchData.symmetry != params.symmetry))) && (searchData.maxCatalysts == params.maxCatalysts || false)) {
            SearchData newData = searchData;
            newData.maxCatalysts++;
            stackLock.lock();
            searchDataStack.push(std::move(newData));
            stackLock.unlock();
            iterativeSearchTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - methodStartTime)).count();
            return;
        }*/

        //if we have an interesting/oscillating/any output, add to categoryContainer/oscillatorCategoryContainer/allOutputsCategoryContainer
        if (searchData.numCatalysts >= params.minCatalysts && searchData.numTransparentCatalysts >= params.minTransparentCatalysts && searchData.numSacrificialCatalysts >= params.minSacrificialCatalysts) {
            if (isValid && params.outputAll) {
                //sample corresponeds to what the value would be at maxGeneration
                int sampleGenerationIndex = maxGenerationAllowed - searchData.generation;
                if (eventualPeriod > 0) {
                    sampleGenerationIndex = stateEvolution.size() - eventualPeriod + ((maxGenerationAllowed - searchData.generation - stateEvolution.size()) % eventualPeriod);
                }
                LifeState diffState = stateEvolution[sampleGenerationIndex];
                diffState.Copy(searchData.startState, XOR);
                categoryContainerLock.lock();
                allOutputsCategoryContainer.Add(searchData.startState, stateEvolution[sampleGenerationIndex], CatContainerKey(diffState, searchData.matchState, 0));
                categoryContainerLock.unlock();
            }
            if (eventualPeriod > 2) {
                int sampleGenerationIndex = stateEvolution.size() - eventualPeriod + ((maxGenerationAllowed - searchData.generation - stateEvolution.size()) % eventualPeriod);
                if (AddOscillatorsToCollection(stateEvolution[sampleGenerationIndex], eventualPeriod)) {
                    LifeState diffState = stateEvolution[sampleGenerationIndex];
                    diffState.Copy(searchData.startState, XOR);
                    
                    categoryContainerLock.lock();
                    oscillatorCategoryContainer.Add(searchData.startState, stateEvolution[sampleGenerationIndex], CatContainerKey(diffState, searchData.matchState, eventualPeriod));
                    categoryContainerLock.unlock();
                }
            }
            if (params.findPartials && !newMatchPoints.empty()) {
                //TODO: Make it so that 'same but originating earlier' cases are placed the same
                //Instead it should be based on the actual supposed period
                for (auto &matchPoint : newMatchPoints) {
                    bool failure = false;
                    LifeState testState = stateEvolution[std::get<0>(matchPoint) - searchData.generation];
                    std::vector<std::vector<unsigned>> catData = catalystsEvolution[std::get<0>(matchPoint) - searchData.generation];
                    //require catalysts to survive for a few more generations
                    for (unsigned i = 0; i < params.partialCatsMustSurviveGens; i++) {
                        testState.Step();
                        for (unsigned catID = 0; catID < catData.size(); catID++) {
                            std::pair<bool, LifeState> filtering = catalysts[catData[catID][0]].CheckState(testState, catData[catID][1], catData[catID][2]);
                            if (!filtering.first) {
                                failure = true;
                            }
                            else if (filtering.second.IsEmpty()) {
                                catData[catID][3] = 0;
                            }
                            else {
                                catData[catID][3]++;
                                if (catData[catID][3] > catalysts[catData[catID][0]].recoveryTime) {
                                    failure = true;
                                }
                            }

                            if (failure) break;
                        }
                        if (failure) break;
                    }
                    if (!failure) {
                        LifeState transformedState = searchData.matchState;
                        transformedState.Transform(std::get<3>(matchPoint));
                        transformedState.Move(std::get<1>(matchPoint), std::get<2>(matchPoint));
                        categoryContainerLock.lock();
                        categoryContainer.Add(searchData.startState, stateEvolution[std::get<0>(matchPoint) - searchData.generation], CatContainerKey(transformedState, searchData.matchState, (int)std::get<0>(matchPoint)));
                        categoryContainerLock.unlock();
                        break;
                    }
                }
            }
        }

        if (searchData.numCatalysts < searchData.maxCatalysts || searchSymmetries) {
            std::stack<SearchData> dataToSearch;

            StaticSymmetry currentSymmetry = GetPatternSymmetry(searchData.currentState, SymmetryGroupFromEnum(searchData.maxSymmetry));
            std::vector<SymmetryTransform> currentSymGroup = SymmetryGroupFromEnum(currentSymmetry);
            std::vector<SymmetryTransform> currentSymChain = SymmetryChainFromEnum(currentSymmetry);

            //there is one issue with the filtering as is: If the active region is deflected away from the catalyst so that it never interacts, we're effectively retreading old ground
            //This only happens if both catalysts are placed with filtering, but for different filters
            //Namely cat 0 must fail, cat 1 must fail separately (outside the filtering region), future cats must deflect active region away from cat 1 entirely
            //If lastCatGenOffset > 0 && useFilter && (the filter has cells outside the old filters)
            //  if cat 2 not in the past of cat 1 (generationwise, not conewise), or if it's in the past light cone of cat 0 failure, we can act as normal
            //  else add (cat 2, cat 1) to the possibilities in the cat 0 search
            //    well, add cat 2 and have a list of approved catalysts
            //  this would also need to be extended to cases with more than 3 catalysts
            //Efficiency gains from this would likely be marginal and it sounds like a nightmare to implement so I probably won't
            bool useFilter = !filterState.IsEmpty();
            //useFilter = false; //decomment this to make sure it runs the same as without filters
            std::vector<LifeState> filterStates; //note: This is reverse order
            if (useFilter) {
                filterStates = {filterState};
                //symmetrize filter if pattern is symmetric, this avoids skipping results
                filterStates[0].SymChain(currentSymChain);
                for (int index = searchData.generation + stateEvolution.size() - 1; index > (int)searchData.generation; index--) {
                    filterStates.push_back(filterStates[filterStates.size() - 1].ZOI());
                }
            }

            std::vector<std::vector<LifeState>> olderFilterStates;
            for (auto [filterState, filterGeneration, filterCatIndex, filterCatX, filterCatY, filterOffsetX, filterOffsetY] : searchData.filters) {
                olderFilterStates.push_back({filterState});
                //symmetrize filters if pattern is symmetric
                olderFilterStates[olderFilterStates.size() - 1][0].SymChain(currentSymChain); //not really a chain but whatever
                for (int index = filterGeneration; index > (int)searchData.generation; index--) {
                    olderFilterStates[olderFilterStates.size() - 1].push_back(olderFilterStates[olderFilterStates.size() - 1][olderFilterStates[olderFilterStates.size() - 1].size() - 1].ZOI());
                }
            }

            std::set<std::vector<LifeState>> possiblePerturbations;

            unsigned maxGenerationToTest = (searchData.numCatalysts == 0 && searchData.symmetry == params.symmetry) ? (std::min(params.maxFirstInteractionGeneration, (unsigned)past1NeighborEvolution.size() - 1 + searchData.generation) - searchData.generation) : (past1NeighborEvolution.size() - 1);
            for (unsigned generationIndex = params.minInteractionGeneration - std::min(searchData.generation, params.minInteractionGeneration); generationIndex < maxGenerationToTest; generationIndex++) {
                //going back 32 or more generations always covers everything, so if we have a filter this should still be at least generationIndex - 32                   
                unsigned nextUseGenerationIndex = useFilter ? std::min(generationIndex, std::max(generationIndex, 32U) - 32U) : generationIndex;
            
                std::chrono::time_point<std::chrono::steady_clock> isolatedStartTime = std::chrono::steady_clock::now();
                
                if (searchData.numCatalysts < searchData.maxCatalysts && (searchData.symmetry != params.symmetry || searchData.numCatalysts < params.maxCatalystsBeforeSymmetry) && (searchData.numCatalysts > 0 || generationIndex + searchData.generation < params.maxFirstCatalystInteractionGeneration)) {
                    unsigned catIndex = 0;
                    unsigned x = 0;
                    unsigned y = 0;
                    if (generationIndex == 0) {
                        catIndex = searchData.startCatIndex;
                        x = searchData.startCatX;
                        y = searchData.startCatY;
                    }

                    for (; catIndex < (unsigned)catalysts.size(); catIndex++) {
                        //catalyst invalid due to
                        if (
                            //too many transparent
                            (catalysts[catIndex].transparent && searchData.numTransparentCatalysts >= params.maxTransparentCatalysts) || 
                            //too few transparent
                            (!catalysts[catIndex].transparent && params.minTransparentCatalysts + searchData.numCatalysts - searchData.numTransparentCatalysts >= searchData.maxCatalysts) ||
                            //too many sacrificial
                            (catalysts[catIndex].sacrificial && searchData.numSacrificialCatalysts >= params.maxSacrificialCatalysts) || 
                            //too few sacrificial
                            (!catalysts[catIndex].sacrificial && params.minSacrificialCatalysts + searchData.numCatalysts - searchData.numSacrificialCatalysts >= searchData.maxCatalysts) ||
                            //too many slots
                            catalysts[catIndex].slots + searchData.occupiedSlots > params.maxSlots ||
                            //redundant under symmetry
                            (!catalysts[catIndex].FirstUnderSymmetry(currentSymGroup))
                        ) {x = 0; y = 0; continue; }
                    
                        //Note: This method does not check cases where two catalysts placed at once are required to interact
                        //I'm not sure how to fix it but it's a bit of an edge case anyways
                        //catalysts must be interacting with the pattern for the first time
                            //must be overlap in one of:
                                //cat1Neighbor with pat2Neighbors
                                //cat2Neighbors with pat1Neighbor
                                //catZOI with pat3Neighbors
                            //must not be overlap in:
                                //catZOI with pat3NeighborsPrevGen
                                //cat1Neighbor with pat2NeighborsPrevGen
                                //cat2Neighbors with pat1NeighborPrevGen
                        LifeState catalystPositions = catalysts[catIndex].locusOneNeighborR180.Convolve(past2NeighborsEvolution[generationIndex + 1]);
                        catalystPositions.Join(catalysts[catIndex].locusTwoNeighborsR180.Convolve(past1NeighborEvolution[generationIndex + 1]));
                        catalystPositions.Join(catalysts[catIndex].locusZoiR180.Convolve(past3NeighborsEvolution[generationIndex + 1]));
                        catalystPositions.Copy(catalysts[catIndex].zoiR180.Convolve(past3NeighborsEvolution[generationIndex]), ANDNOT);
                        if (catalystPositions.IsEmpty()) {x = 0; y = 0; continue;}
                        catalystPositions.Copy(catalysts[catIndex].oneNeighborR180.Convolve(past2NeighborsEvolution[generationIndex]), ANDNOT);
                        if (catalystPositions.IsEmpty()) {x = 0; y = 0; continue;}
                        catalystPositions.Copy(catalysts[catIndex].twoNeighborsR180.Convolve(past1NeighborEvolution[generationIndex]), ANDNOT);
                        if (catalystPositions.IsEmpty()) {x = 0; y = 0; continue;}

                        //extra criterion for filtering
                        if (useFilter) {
                            catalystPositions.Copy(catalysts[catIndex].zoiR180.Convolve(filterStates[stateEvolution.size() - 1 - generationIndex]), AND);
                            if (catalystPositions.IsEmpty()) {x = 0; y = 0; continue;}
                        }
                        //filter out stuff that was already checked in a stored filter
                        int oldFilterIndex = 0;
                        bool breakFromOldFilters = false;
                        for (auto [filterState, filterGeneration, filterCatIndex, filterCatX, filterCatY, filterOffsetX, filterOffsetY] : searchData.filters) {
                            if (generationIndex + searchData.generation < filterGeneration || (generationIndex + searchData.generation == filterGeneration && catIndex <= filterCatIndex)) {
                                LifeState excludedCatalysts = catalysts[catIndex].zoiR180.Convolve(olderFilterStates[oldFilterIndex][filterGeneration - (generationIndex + searchData.generation)]);
                                if (generationIndex + searchData.generation == filterGeneration && catIndex == filterCatIndex) {
                                    //some x and y aren't excluded yet
                                    for (unsigned filterCatYTemp = filterCatY + 1; filterCatYTemp < 64; filterCatYTemp++) {
                                        excludedCatalysts.SetCell((filterCatX + filterOffsetX) % 64, (filterCatYTemp + filterOffsetY) % 64, 0);
                                    }
                                    for (unsigned filterCatXTemp = filterCatX + 1; filterCatXTemp < 64; filterCatXTemp++) {
                                        excludedCatalysts.state[(filterCatXTemp + filterOffsetX) % 64] = 0ULL;
                                    }
                                }
                                catalystPositions.Copy(std::move(excludedCatalysts), ANDNOT);
                                if (catalystPositions.IsEmpty()) {x = 0; y = 0; breakFromOldFilters = true; break;}
                            }
                            oldFilterIndex++;
                        }
                        if (breakFromOldFilters) continue;

                        //neither of these can make catalystPositions empty so we ignore the 'continue if it's empty' thing
                        //apply exclusions to prevent ambiguity when cat and pat share symmetry
                        //TODO: With multiple catalysts being added in the same generation, this seems to not work perfectly?
                        if (currentSymmetry != C1)
                            catalystPositions.Copy(catalysts[catIndex].fundamentalDomains[static_cast<int>(currentSymmetry)], AND);

                        //symmetry exclusions
                        if (searchData.symmetry != C1)
                            catalystPositions.Copy(catalysts[catIndex].symmetryExclusions[static_cast<int>(searchData.symmetry)], ANDNOT);

                        for (; x < 64; x++) {
                            if (catalystPositions.state[x] != 0ULL) {
                                for (; y < 64; y++) {
                                    if (catalystPositions.GetCell(x, y) == 1) {
                                        //exclude forbidden placements
                                        //TODO: This could maybe use a streamlined version that only checks brokenness
                                        //(Maybe even just forbidden and antirequired)
                                        LifeState catStateSymChain = catalysts[catIndex].state.GetSymChain(x, y, SymmetryChainFromEnum(searchData.symmetry));
                                        LifeState newCurrentState = stateEvolution[nextUseGenerationIndex];
                                        newCurrentState.Join(catStateSymChain);
                                        if (!catalysts[catIndex].CheckState(newCurrentState, x, y).first) {
                                            continue;
                                        }
                                        //checkRecovery catalysts
                                        //TODO: Maybe we could also (optionally) exclude catalysts that end up changing nothing?
                                        if (catalysts[catIndex].checkRecovery) {
                                            LifeState testState = stateEvolution[generationIndex];
                                            std::vector<LifeState> perturbation;
                                            testState.Join(catStateSymChain);
                                            bool recovered = false;
                                            //TODO: This may not need to be tested if it goes beyond params.maxGeneration
                                            for (int extraGen = 0; extraGen < (int)catalysts[catIndex].recoveryTime; extraGen++) {
                                                testState.Step();

                                                if (params.checkRecoveryRequireUnique) {
                                                    perturbation.push_back(testState);
                                                    perturbation[perturbation.size() - 1].Copy(catStateSymChain, XOR);
                                                }

                                                std::pair<bool, LifeState> filtering = catalysts[catIndex].CheckState(testState, x, y);
                                                if (!filtering.first) {
                                                    //catalyst breaks
                                                    break;
                                                }
                                                else if (filtering.second.IsEmpty()) {
                                                    recovered = true;
                                                    break;
                                                }
                                            }
                                            if (!recovered) continue;
                                            //having this means we only check one representative of each perturbation
                                            //Note: This is currently checking the entire perturbation history: For example, eater 1 and eater 2 would be distinct
                                            if (params.checkRecoveryRequireUnique) {
                                                if (possiblePerturbations.find(perturbation) != possiblePerturbations.end()) continue;
                                                possiblePerturbations.insert(std::move(perturbation));
                                            }
                                        }

                                        //construct new data and add to stack
                                        SearchData newData;
                                        newData.maxCatalysts = searchData.maxCatalysts;
                                        newData.generation = nextUseGenerationIndex + searchData.generation;
                                        newData.numCatalysts = searchData.numCatalysts + 1;
                                        newData.occupiedSlots = searchData.occupiedSlots + catalysts[catIndex].slots;
                                        newData.lastCatGenOffset = generationIndex - nextUseGenerationIndex;
                                        newData.lastAddedWasCatalyst = true;

                                        newData.symmetry = searchData.symmetry;
                                        newData.maxSymmetry = searchData.maxSymmetry;

                                        newData.numTransparentCatalysts = searchData.numTransparentCatalysts;
                                        if (catalysts[catIndex].transparent) newData.numTransparentCatalysts++;
                                        newData.numSacrificialCatalysts = searchData.numSacrificialCatalysts;
                                        if (catalysts[catIndex].sacrificial) newData.numSacrificialCatalysts++;

                                        if (useFilter) {
                                            newData.startCatIndex = searchData.startCatIndex;
                                            newData.startCatX = searchData.startCatX;
                                            newData.startCatY = searchData.startCatY;
                                            newData.transformLowerBound = searchData.transformLowerBound;
                                        }
                                        else {
                                            //catalysts are lex-after transforms of the same generation
                                            newData.startCatIndex = catIndex;
                                            newData.startCatX = x;
                                            newData.startCatY = y + 1;
                                            newData.transformLowerBound = NUM_TRANSFORMS;
                                        }

                                        newData.startState = (searchData.numCatalysts == 0 && params.patternRand && false) ? stateEvolution[nextUseGenerationIndex] : searchData.startState;
                                        newData.currentState = std::move(newCurrentState);
                                        newData.past1Neighbor = past1NeighborEvolution[nextUseGenerationIndex];
                                        newData.past2Neighbors = past2NeighborsEvolution[nextUseGenerationIndex];
                                        newData.past3Neighbors = past3NeighborsEvolution[nextUseGenerationIndex];
                                        newData.catalysts = catalystsEvolution[nextUseGenerationIndex];

                                        //Update newData with the new catalyst
                                        //note: in the asymmetric case, this can be done marginally more efficiently with caching
                                        //I mean you could probably also cache in the symmetric case for every possible placement but that would take a lot of memory (~46 MB per catalyst)
                                        LifeState cat1NeighborSymChain = catStateSymChain.OneNeighbor();
                                        LifeState cat2NeighborsSymChain = catStateSymChain.TwoNeighbors();
                                        LifeState catZOISymChain = catStateSymChain.ZOI();

                                        LifeState past1Cat1 = newData.past1Neighbor;
                                        past1Cat1.Copy(cat1NeighborSymChain, AND);
                                        LifeState past1Cat2 = newData.past1Neighbor;
                                        past1Cat2.Copy(cat2NeighborsSymChain, AND);
                                        LifeState past2Cat1 = newData.past2Neighbors;
                                        past2Cat1.Copy(cat1NeighborSymChain, AND);

                                        newData.startState.Join(catStateSymChain);

                                        newData.past1Neighbor.Copy(catZOISymChain, ANDNOT);
                                        newData.past1Neighbor.Join(std::move(cat1NeighborSymChain));

                                        newData.past2Neighbors.Copy(catZOISymChain, ANDNOT);
                                        newData.past2Neighbors.Join(std::move(past1Cat1));
                                        newData.past2Neighbors.Join(std::move(cat2NeighborsSymChain));
                                        
                                        newData.past3Neighbors.Copy(std::move(catZOISymChain), ANDNOT);
                                        newData.past3Neighbors.Join(std::move(past1Cat2));
                                        newData.past3Neighbors.Join(std::move(past2Cat1));
                                        newData.past3Neighbors.Join(std::move(catStateSymChain)); //for still lives state is threeNeighbors

                                        newData.catalysts.push_back({catIndex, x, y, 0});

                                        //re-add all still applicable filters
                                        for (auto &filterData : searchData.filters) {
                                            if (std::get<1>(filterData) > newData.generation || 
                                                (std::get<1>(filterData) == newData.generation && (
                                                    std::get<2>(filterData) > newData.startCatIndex ||
                                                    (std::get<2>(filterData) == newData.startCatIndex && (
                                                        std::get<3>(filterData) > newData.startCatX ||
                                                        (std::get<3>(filterData) == newData.startCatX && 
                                                            std::get<4>(filterData) > newData.startCatY)
                                                    ))
                                                ))
                                                ) {
                                                newData.filters.push_back(filterData);
                                            }
                                        }
                                        //update filters
                                        if (useFilter) {
                                            newData.filters.push_back({filterStates[stateEvolution.size() - 1 - generationIndex], searchData.generation + generationIndex, catIndex, x, y, 0, 0});
                                        }

                                        if (params.findPartials) {
                                            newData.matchState = (searchData.numCatalysts == 0 && params.patternRand) ? stateEvolution[nextUseGenerationIndex] : searchData.matchState;                                        
                                            newData.matchPoints = newMatchPoints;
                                            //re-add applicable match points
                                            for (auto &matchPointData : searchData.matchPoints) {
                                                if (std::get<0>(matchPointData) > newData.generation) {
                                                    newData.matchPoints.push_back(matchPointData);
                                                }
                                            }
                                        }

                                        dataToSearch.push(std::move(newData));
                                    }
                                }
                            }
                            y = 0;
                        }
                        x = 0;
                    }
                }
                if (searchSymmetries && searchData.numCatalysts >= params.minCatalystsBeforeSymmetry && (generationIndex + searchData.generation == 0 || params.doDelayedSymmetryInteraction)) {
                    //Note: Using an exclusion zones system here MIGHT be faster, but it'd be a pain to implement and also result in errors, and it probably still wouldn't be faster
                    LifeState newNeighbors = past1NeighborEvolution[generationIndex + 1];
                    newNeighbors.Join(past2NeighborsEvolution[generationIndex + 1]);
                    newNeighbors.Join(past3NeighborsEvolution[generationIndex + 1]);
                    LifeState oldNeighbors;
                    //allow symmetric region overlap on generation 0 with no catalysts
                    if (searchData.generation > 0 || searchData.numCatalysts > 0 || generationIndex > 0 || !params.patternRand) {
                        oldNeighbors = past1NeighborEvolution[generationIndex];
                        oldNeighbors.Join(past2NeighborsEvolution[generationIndex]);
                        oldNeighbors.Join(past3NeighborsEvolution[generationIndex]);
                    }
                    //TODO: It'd be nice for identical catalysts to be able to overlap
                    
                    LifeState newNeighborsFiltered = newNeighbors;
                    if (useFilter) {
                        //TODO: This may be a bit too large? Better too large than too small, but needs testing
                        newNeighborsFiltered.Copy(filterStates[past1NeighborEvolution.size() - 1 - generationIndex], AND);
                    }

                    unsigned transformLowerBound = 0;
                    if (generationIndex == 0) transformLowerBound = searchData.transformLowerBound;
                    for (auto [newSymmetry, generator] : availableSymmetries) {
                        if (static_cast<unsigned>(generator) > transformLowerBound) {
                            //Don't double-count if the pattern is symmetric: Certain exclusions can be made right away
                            bool skipSymmetry = false;
                            switch (generator) {
                                case ReflectAcrossY:
                                case ReflectAcrossYEven:
                                    switch (currentSymmetry) {
                                        //already covered by ReflectAcrossX
                                        case D2diagodd:
                                        case D2negdiagodd:
                                        case D2negdiagevenSubgroupsOnly:
                                        case D4diag:
                                        case D4diageven:
                                        case D8:
                                        case D8even:
                                            skipSymmetry = true;
                                            break;
                                        default:
                                            break;
                                    }
                                    break;
                                case ReflectAcrossYeqNegX:
                                case ReflectAcrossYeqNegXP1:
                                    switch (currentSymmetry) {
                                        //already covered by ReflectAcrossYeqX
                                        case D2AcrossX:
                                        case D2AcrossXEven:
                                        case D2AcrossY:
                                        case D2AcrossYEven:
                                        case D4:
                                        case D4even:
                                        case D4horizontaleven:
                                        case D4verticaleven:
                                        case D8:
                                        case D8even:
                                            skipSymmetry = true;
                                            break;
                                        default:
                                            break;
                                    }
                                    break;
                                default:
                                    break;
                            }
                            if (skipSymmetry) continue;

                            std::vector<SymmetryTransform> generationChain = {generator};
                            switch (newSymmetry) {
                                case C4: case D8: generationChain.push_back(Rotate180OddBoth); break;
                                case C4even: case D8even: generationChain.push_back(Rotate180EvenBoth); break;
                                default: break;
                            }

                            //convolve-based valid transform positions
                            LifeState transformPositions = FilteredTransformPositions(newNeighbors, newNeighborsFiltered, generator); //only use the first generator since other interactions wouldn't be allowed anyway
                            
                            if (useFilter) {
                                switch (generator) {
                                    case Rotate90:
                                    case Rotate90Even:
                                    case Rotate270:
                                    case Rotate270Even:
                                        transformPositions.Join(FilteredTransformPositions(newNeighborsFiltered, newNeighbors, generator));
                                        break;
                                    default: break;
                                }
                            }
                            bool noAvailablePositions = false;
                            for (auto &chainGenerator : generationChain) {
                                transformPositions.Copy(TransformPositions(oldNeighbors, chainGenerator), ANDNOT);
                                if (transformPositions.IsEmpty()) {
                                    noAvailablePositions = true;
                                    break;
                                }
                            }
                            if (noAvailablePositions) continue;

                            //prevent from using a transform that was filtered previously
                            //TODO: I should definitely double-check that this isn't either removing too little or too much at some point
                            int oldFilterIndex = 0;
                            for (auto [filterState, filterGeneration, filterCatIndex, filterCatX, filterCatY, filterOffsetX, filterOffsetY] : searchData.filters) {
                                if (generationIndex + searchData.generation <= filterGeneration) {
                                    LifeState filterStateInflated = olderFilterStates[oldFilterIndex][filterGeneration - (generationIndex + searchData.generation)];
                                    
                                    LifeState oldNewNeighborsFiltered = newNeighbors;
                                    oldNewNeighborsFiltered.Copy(filterStateInflated, AND);

                                    LifeState excludedTransforms = FilteredTransformPositions(newNeighbors, oldNewNeighborsFiltered, generator);
                                    switch (generator) {
                                        case Rotate90:
                                        case Rotate90Even:
                                        case Rotate270:
                                        case Rotate270Even:
                                            excludedTransforms.Join(FilteredTransformPositions(oldNewNeighborsFiltered, newNeighbors, generator));
                                            break;
                                        default: break;
                                    }

                                    transformPositions.Copy(std::move(excludedTransforms), ANDNOT);
                                    if (transformPositions.IsEmpty()) {noAvailablePositions = true; break;}
                                }
                                oldFilterIndex++;
                            }
                            if (noAvailablePositions) continue;

                            //don't double-count if the pattern is symmetric
                            bool sharedXSymmetry = false;
                            bool sharedYSymmetry = false;
                            switch(generator) {
                                case ReflectAcrossX:
                                case ReflectAcrossXEven:
                                    switch (currentSymmetry) {
                                        case D2AcrossX:
                                        case D2AcrossXEven:
                                        case D2diagodd:
                                        case D2negdiagevenSubgroupsOnly:
                                        case D2negdiagodd:
                                        case C2:
                                        case C2even:
                                        case C2horizontaleven:
                                        case C2verticaleven:
                                        case C4:
                                        case C4even:
                                        case D4:
                                        case D4even:
                                        case D4verticaleven:
                                        case D4horizontaleven:
                                        case D4diag:
                                        case D4diageven:
                                        case D8:
                                        case D8even:
                                            sharedXSymmetry = true;
                                            break;
                                        default: break;
                                    }
                                    break;
                                case ReflectAcrossY:
                                case ReflectAcrossYEven:
                                    switch (currentSymmetry) {
                                        case D2AcrossY:
                                        case D2AcrossYEven:
                                        case D2diagodd:
                                        case D2negdiagevenSubgroupsOnly:
                                        case D2negdiagodd:
                                        case C2:
                                        case C2even:
                                        case C2horizontaleven:
                                        case C2verticaleven:
                                        case C4:
                                        case C4even:
                                        case D4:
                                        case D4even:
                                        case D4verticaleven:
                                        case D4horizontaleven:
                                        case D4diag:
                                        case D4diageven:
                                        case D8:
                                        case D8even:
                                            sharedYSymmetry = true;
                                            break;
                                        default: break;
                                    }
                                    break;
                                case ReflectAcrossYeqX:
                                    switch (currentSymmetry) {
                                        case D2diagodd:
                                        case D2AcrossX:
                                        case D2AcrossXEven:
                                        case D2AcrossY:
                                        case D2AcrossYEven:
                                        case C2:
                                        case C2even:
                                        case C2horizontaleven:
                                        case C2verticaleven:
                                        case C4:
                                        case C4even:
                                        case D4:
                                        case D4even:
                                        case D4verticaleven:
                                        case D4horizontaleven:
                                        case D4diag:
                                        case D4diageven:
                                        case D8:
                                        case D8even:
                                            sharedYSymmetry = true;
                                            break;
                                        default: break;
                                    }
                                    break;
                                case ReflectAcrossYeqNegX:
                                case ReflectAcrossYeqNegXP1:
                                    switch (currentSymmetry) {
                                        case D2negdiagevenSubgroupsOnly:
                                        case D2negdiagodd:
                                        case D2AcrossX:
                                        case D2AcrossXEven:
                                        case D2AcrossY:
                                        case D2AcrossYEven:
                                        case C2:
                                        case C2even:
                                        case C2horizontaleven:
                                        case C2verticaleven:
                                        case C4:
                                        case C4even:
                                        case D4:
                                        case D4even:
                                        case D4verticaleven:
                                        case D4horizontaleven:
                                        case D4diag:
                                        case D4diageven:
                                        case D8:
                                        case D8even:
                                            sharedYSymmetry = true;
                                            break;
                                        default: break;
                                    }
                                    break;
                                case Rotate180EvenBoth:
                                case Rotate180EvenHorizontal:
                                case Rotate180EvenVertical:
                                case Rotate180OddBoth:
                                    switch (currentSymmetry) {
                                        case D2AcrossX:
                                        case D2AcrossXEven:
                                        case C2:
                                        case C2even:
                                        case C2verticaleven:
                                        case C2horizontaleven:
                                        case C4:
                                        case C4even:
                                        case D4diag:
                                        case D4diageven:
                                            sharedXSymmetry = true;
                                            break;
                                        case D2AcrossY:
                                        case D2AcrossYEven:
                                            sharedYSymmetry = true;
                                            break;
                                        case D4:
                                        case D4even:
                                        case D4horizontaleven:
                                        case D4verticaleven:
                                        case D8:
                                        case D8even:
                                            sharedXSymmetry = true;
                                            sharedYSymmetry = true;
                                            break;
                                        default: break;
                                    }
                                    break;
                                case Rotate90:
                                case Rotate90Even:
                                    switch (currentSymmetry) {
                                        case D2AcrossX:
                                        case D2AcrossXEven:
                                        case C2:
                                        case C2even:
                                        case C2verticaleven:
                                        case C2horizontaleven:
                                        case D4diag:
                                        case D4diageven:
                                            sharedXSymmetry = true;
                                            break;
                                        case D2AcrossY:
                                        case D2AcrossYEven:
                                            sharedYSymmetry = true;
                                            break;
                                        case D4:
                                        case D4even:
                                        case D4horizontaleven:
                                        case D4verticaleven:
                                        case C4:
                                        case C4even:
                                        case D8:
                                        case D8even:
                                            sharedXSymmetry = true;
                                            sharedYSymmetry = true;
                                            break;
                                        default: break;
                                    }
                                    break;
                                default: break;
                            }

                            //TODO: Rotation transform with symmetry including diagonal reflection results in copies, should avoid those
                            
                            std::pair<int, int> maxCoords = GetMaxUniqueMovementCoords(generator);
                            //movement is biased towards being closer to the center
                            for (int baseX = sharedYSymmetry ? 0 : -(maxCoords.first / 2); baseX < (maxCoords.first + 1) / 2; baseX++) {
                                if (transformPositions.state[(baseX + 64) % 64] != 0ULL) {
                                    for (int baseY = sharedXSymmetry ? 0 : -(maxCoords.second / 2); baseY < (maxCoords.second + 1) / 2; baseY++) {
                                        if (transformPositions.GetCell(baseX, baseY) == 1) {
                                            int x;
                                            int y;
                                            unsigned xOffset = 0;
                                            unsigned yOffset = 0;
                                            StaticSymmetry useSymmetry = newSymmetry;
                                            //modify positioning to make some things not break
                                            if (newSymmetry == D8 || newSymmetry == D8even) {
                                                switch (searchData.symmetry) {
                                                    case D2AcrossX:
                                                    case D2AcrossXEven:
                                                        x = baseX;
                                                        y = 0;
                                                        break;
                                                    case D2AcrossY:
                                                    case D2AcrossYEven:
                                                    continue;
                                                        x = 0;
                                                        y = baseX;
                                                        break;
                                                    case D2diagodd:
                                                        switch (generator) {
                                                            case ReflectAcrossX:
                                                            case ReflectAcrossXEven:
                                                                x = baseY;
                                                                y = baseY;
                                                                break;
                                                            default:
                                                                x = baseX;
                                                                y = baseX;
                                                                break;
                                                        }
                                                        break;
                                                    case D2negdiagodd:
                                                        switch (generator) {
                                                            case ReflectAcrossX:
                                                                x = -baseY;
                                                                y = baseY;
                                                                break;
                                                            case ReflectAcrossXEven:
                                                                x = -baseY - 1;
                                                                y = baseY;
                                                                break;
                                                            case ReflectAcrossY:
                                                                x = baseX;
                                                                y = -baseX;
                                                                break;
                                                            default:
                                                                x = baseX;
                                                                y = -baseX - 1;
                                                                break;
                                                        }
                                                        break;
                                                    default:
                                                        x = baseX;
                                                        y = baseY;
                                                        break;
                                                }
                                            }
                                            //Move to recenter in the case of D4diag, and possibly switch to D4diageven
                                            else if (newSymmetry == D4diag) {
                                                //baseX ranges from -32 to 31
                                                //baseY = 0
                                                if (generator == ReflectAcrossYeqX) {
                                                    x = ((baseX + 129) / 2) % 64;
                                                    y = (128 - (baseX + 128) / 2) % 64;
                                                }
                                                else {
                                                    x = ((baseX + 128) / 2) % 64;
                                                    y = ((baseX + 129) / 2) % 64;
                                                }
                                                if ((baseX + 128) % 2 == 1) { //D4diageven
                                                    useSymmetry = D4diageven;
                                                    //I don't know why this needs to be here and I hate that it works
                                                    if (generator == ReflectAcrossYeqX) {
                                                        xOffset = -1;
                                                        yOffset = -1;
                                                    }
                                                    else {
                                                        xOffset = 0;
                                                        yOffset = -1;
                                                    }
                                                }
                                            }
                                            else {
                                                x = baseX;
                                                y = baseY;
                                            }
                                            x = (x + 64) % 64;
                                            y = (y + 64) % 64;

                                            //construct new data and add to stack
                                            SearchData newData;
                                            newData.maxCatalysts = searchData.maxCatalysts;
                                            newData.generation = nextUseGenerationIndex + searchData.generation;
                                            newData.numCatalysts = searchData.numCatalysts;
                                            newData.numTransparentCatalysts = searchData.numTransparentCatalysts;
                                            newData.numSacrificialCatalysts = searchData.numSacrificialCatalysts;
                                            newData.occupiedSlots = searchData.occupiedSlots;
                                            newData.lastCatGenOffset = generationIndex - nextUseGenerationIndex;
                                            newData.lastAddedWasCatalyst = false;

                                            if (useFilter) {
                                                newData.startCatIndex = searchData.startCatIndex;
                                                newData.startCatX = searchData.startCatX;
                                                newData.startCatY = searchData.startCatY;
                                                newData.transformLowerBound = searchData.transformLowerBound;
                                            }
                                            else {
                                                //transforms are lex-before catalysts of the same generation
                                                newData.startCatIndex = 0;
                                                newData.startCatX = 0;
                                                newData.startCatY = 0;
                                                newData.transformLowerBound = static_cast<unsigned>(generator);
                                            }

                                            //move + transform these
                                            //note: this being correct requires no overlap between newData.pastNNeighbors
                                            //interaction does require overlap
                                            //(this is not technically sufficient to guarantee interaction but whatever)
                                            newData.startState = searchData.startState.GetSymChain(x, y, generationChain);
                                            newData.startState.Move(xOffset, yOffset);
                                            newData.currentState = stateEvolution[nextUseGenerationIndex].GetSymChain(x, y, generationChain);
                                            newData.currentState.Move(xOffset, yOffset);
                                            newData.past1Neighbor = past1NeighborEvolution[nextUseGenerationIndex].GetSymChain(x, y, generationChain);
                                            newData.past1Neighbor.Move(xOffset, yOffset);
                                            newData.past2Neighbors = past2NeighborsEvolution[nextUseGenerationIndex].GetSymChain(x, y, generationChain);
                                            newData.past2Neighbors.Move(xOffset, yOffset);
                                            newData.past3Neighbors = past3NeighborsEvolution[nextUseGenerationIndex].GetSymChain(x, y, generationChain);
                                            newData.past3Neighbors.Move(xOffset, yOffset);
          
                                            newData.symmetry = useSymmetry;
                                            newData.maxSymmetry = GetPatternSymmetry(newData.startState, allTransforms);

                                            //move catalysts
                                            for (auto &catData : catalystsEvolution[nextUseGenerationIndex]) {
                                                newData.catalysts.push_back({catData[0], (unsigned)((int)catData[1] + x + xOffset + 64) % 64, (unsigned)((int)catData[2] + y + yOffset + 64) % 64, catData[3]});
                                            }

                                            //re-add and move all still applicable stored filters
                                            for (auto &filterData : searchData.filters) {
                                                if (std::get<1>(filterData) >= newData.generation) {
                                                    LifeState newFilterState = std::get<0>(filterData);
                                                    newFilterState.Move(x + xOffset, y + yOffset);
                                                    newData.filters.push_back({
                                                        std::move(newFilterState), 
                                                        std::get<1>(filterData), 
                                                        std::get<2>(filterData),
                                                        std::get<3>(filterData), 
                                                        std::get<4>(filterData),
                                                        (std::get<5>(filterData) + x + xOffset) % 64,
                                                        (std::get<6>(filterData) + y + yOffset) % 64
                                                    });
                                                }
                                            }
                                            //update filters
                                            if (useFilter) {
                                                LifeState newFilterState = filterStates[past1NeighborEvolution.size() - 1 - generationIndex];
                                                newFilterState.Move(x + xOffset, y + yOffset);
                                                newData.filters.push_back({std::move(newFilterState), searchData.generation + generationIndex, 0, 0, 0, (x + xOffset + 64) % 64, (y + yOffset + 64) % 64});
                                            }

                                            if (params.findPartials) {
                                                newData.matchState = searchData.matchState.GetSymChain(x, y, generationChain);
                                                newData.matchState.Move(xOffset, yOffset);
                                                //TODO: Non-identity match points that still work
                                                //re-add applicable match points
                                                for (auto &matchPointData : searchData.matchPoints) {
                                                    if (std::get<0>(matchPointData) > newData.generation && std::get<3>(matchPointData) == Identity) {
                                                        newData.matchPoints.push_back(matchPointData);
                                                    }
                                                }
                                                for (auto &matchPointData : newMatchPoints) {
                                                    if (std::get<3>(matchPointData) == Identity) {
                                                        newData.matchPoints.push_back(matchPointData);
                                                    }
                                                }
                                            }

                                            dataToSearch.push(std::move(newData));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                isolatedTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - isolatedStartTime)).count();
            }

            //push to stack in reverse generation order
            //note: a priority queue *might* be better for searches that I wouldn't expect to terminate
            //This would require a proper heuristic (seems like it'd maybe be a good place to use ML, which I unfortunately don't know)
            //Being slower they're still worse for terminating searches
            stackLock.lock();
            while (!dataToSearch.empty()) {
                searchDataStack.push(std::move(dataToSearch.top()));
                dataToSearch.pop();
            }
            stackLock.unlock();
        }

        iterativeSearchTime += ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - methodStartTime)).count();
    }

    std::vector<std::pair<StaticSymmetry, SymmetryTransform>> GetAvailableSymmetries(const StaticSymmetry &sym) {
        std::vector<std::pair<StaticSymmetry, SymmetryTransform>> output;
        for (auto &symmetryTransformPair : SymmetryImmediateSuccessorsOf(sym)) {
            if (params.possibleSymmetries.find(symmetryTransformPair.first) != params.possibleSymmetries.end()) output.push_back(symmetryTransformPair);
        }
        return output;
    }

    void CheckMatches(std::vector<std::tuple<unsigned, unsigned, unsigned, SymmetryTransform>> *newMatchPoints, const std::vector<std::tuple<unsigned, unsigned, unsigned, SymmetryTransform>> &oldMatchPoints, const LifeState &testState, const LifeState &matchState, const LifeState &antiMatchState, const LifeState &avoidState, const unsigned &generation, const unsigned &maxXOffset, const unsigned &maxYOffset, const SymmetryTransform &transform, const int &diagonality = 0) {
        LifeState transformedMatchState = matchState;
        transformedMatchState.Transform(transform);
        LifeState transformedAntiMatchState = antiMatchState;
        transformedAntiMatchState.Transform(transform);

        if (maxXOffset == 64 && maxYOffset == 64) {
            //pattern-match everywhere at once if we're testing everywhere
            LifeState matches = transformedMatchState.FindMatches(testState);
            for (unsigned x = 0; x < maxXOffset; x++) {
                if (matches.state[x] != 0)
                    for (unsigned y = 0; y < maxYOffset; y++) {
                        if (matches.GetCell(x, y) == 1) {
                            if (std::find(oldMatchPoints.begin(), oldMatchPoints.end(), std::tuple<unsigned, unsigned, unsigned, SymmetryTransform>(generation, x, y, transform)) == oldMatchPoints.end())
                                if (testState.AreDisjoint(transformedAntiMatchState, x, y) && avoidState.AreDisjoint(transformedMatchState, x, y))
                                    newMatchPoints->push_back({generation, x, y, transform});
                        }
                    }
            }
        } else {
            //otherwise just checking each cell individually is faster
            for (unsigned x = 0; x < maxXOffset; x++) {
                for (unsigned yIndex = 0; yIndex < maxYOffset; yIndex++) {
                    unsigned y = (yIndex + (64 + diagonality) * x) % 64;
                    if (std::find(oldMatchPoints.begin(), oldMatchPoints.end(), std::tuple<unsigned, unsigned, unsigned, SymmetryTransform>(generation, x, y, transform)) == oldMatchPoints.end())
                        if (testState.Contains(transformedMatchState, x, y) && testState.AreDisjoint(transformedAntiMatchState, x, y) && avoidState.AreDisjoint(transformedMatchState, x, y))
                            newMatchPoints->push_back({generation, x, y, transform});
                }
            }
        }
    }

    LifeState TransformPositions(const LifeState &state, const SymmetryTransform &transform) {
        LifeState transformedState = state;
        transformedState.Transform(transform);
        transformedState.Transform(Rotate180OddBoth);
        LifeState transformSelfOverlapPositions = state.Convolve(transformedState);

        LifeState output;
        for (unsigned x = 0; x < 64; x++) {
            for (unsigned y = 0; y < 64; y++) {
                std::pair<int, int> xy = MovedTransformedOffsetFromTransformedMoved(x, y, transform);
                if (transformSelfOverlapPositions.GetCell(xy.first % 64, xy.second % 64) == 1) {
                    output.SetCell(x, y, 1);
                }
            }
        }
        return output;
    }

    LifeState FilteredTransformPositions(const LifeState &state, const LifeState &filteredState, const SymmetryTransform &transform) {
        LifeState transformedState = state;
        transformedState.Transform(transform);
        transformedState.Transform(Rotate180OddBoth);
        LifeState transformSelfOverlapPositions = filteredState.Convolve(transformedState);

        LifeState output;
        for (unsigned x = 0; x < 64; x++) {
            for (unsigned y = 0; y < 64; y++) {
                std::pair<int, int> xy = MovedTransformedOffsetFromTransformedMoved(x, y, transform);
                if (transformSelfOverlapPositions.GetCell(xy.first % 64, xy.second % 64) == 1) {
                    output.SetCell(x, y, 1);
                }
            }
        }
        return output;
    }

    std::pair<int, int> MovedTransformedOffsetFromTransformedMoved(const int &x, const int &y, const SymmetryTransform &transform) {
        switch (transform) {
            case Identity: return {0, 0};
            case ReflectAcrossX: 
            case ReflectAcrossXEven: return {0, -2 * y};
            case ReflectAcrossY: 
            case ReflectAcrossYEven: return {-2 * x, 0};
            case Rotate90:
            case Rotate90Even: return {-y - x, x - y};
            case Rotate270:
            case Rotate270Even: return {y - x, -x - y};
            case Rotate180OddBoth:
            case Rotate180EvenBoth:
            case Rotate180EvenHorizontal:
            case Rotate180EvenVertical: return {-2 * x, -2 * y};
            case ReflectAcrossYeqX: return {y - x, x - y};
            case ReflectAcrossYeqNegX:
            case ReflectAcrossYeqNegXP1: return {-y - x, -x - y};
            default: return {0, 0};
        }
    }

    bool AddOscillatorsToCollection(const LifeState &state, const unsigned &period) {
        //add all oscillators of period > 2
        LifeState stateCopy = state;
        bool foundNewOsc = false;
        for (unsigned x = 0; x < 64; x++) {
            if (stateCopy.state[x] != 0ULL) {
                for (unsigned y = 0; y < 64; y++) {
                    if (stateCopy.GetCell(x, y) == 1) {
                        //need to flood every cell based on evolution impact
                        LifeState oscMask;
                        oscMask.SetCell(x, y, 1);
                        unsigned gen = 0;
                        unsigned lastGenUpated = 0;
                        LifeState currentState = stateCopy;
                        while (lastGenUpated + period * 2 >= gen) {
                            gen++;
                            LifeState testStateNotOsc = currentState;
                            testStateNotOsc.Copy(oscMask, ANDNOT);
                            LifeState testStateOsc = currentState;
                            testStateOsc.Copy(oscMask, AND);
                            currentState.Step();
                            testStateNotOsc.Step();
                            testStateOsc.Step();
                            LifeState testState = testStateOsc;
                            testState.Join(testStateNotOsc);
                            
                            //patterns affect each other
                            //expand oscMask
                            //this must expand to contain:
                            //  any active cell in the ZOI of the current oscMask
                            //  any cell which is on in either osc or not osc but not in currentState
                            //TODO: This may not quite work perfectly if multi-unices are any indication
                            LifeState newOscMaskCells = oscMask.ZOI();
                            newOscMaskCells.Copy(currentState, AND);
                            LifeState newOscMaskCellsFromErrors = testState;
                            newOscMaskCellsFromErrors.Copy(currentState, ANDNOT);
                            newOscMaskCells.Join(newOscMaskCellsFromErrors);
                            if (!oscMask.Contains(newOscMaskCells)) lastGenUpated = gen;
                            oscMask.Join(newOscMaskCells);
                        }
                        LifeState isolatedOsc = stateCopy;
                        isolatedOsc.Copy(oscMask, AND);
                        foundNewOsc |= AddOscillatorToCollection(std::move(isolatedOsc));

                        stateCopy.Copy(std::move(oscMask), ANDNOT);
                    }
                }
            }
        }
        return foundNewOsc;
    }

    bool AddOscillatorToCollection(const LifeState &oscillator) {
        unsigned period = 0;
        LifeState copy = oscillator;
        do {
            period++;
            copy.Step();
        } while (!(copy == oscillator));
        if (period > 2) {
            LifeState standardizedState = std::get<0>(copy.StandardizedWithTransforms(C1));
            for (unsigned i = 1; i < period; i++) {
                copy.Step();

                standardizedState = std::min(standardizedState, std::get<0>(copy.StandardizedWithTransforms(C1)));
            }

            avoidOscContainerLock.lock();
            if (avoidOscs.find(standardizedState) == avoidOscs.end()) {
                avoidOscs.insert(std::move(standardizedState));
                avoidOscContainerLock.unlock();
                return true;
            }
            else {
                avoidOscContainerLock.unlock();
                return false;
            }
        }
        else return false;
    }

    void Report() {
        printLock.lock();
        std::chrono::duration<double> timePassed = std::chrono::steady_clock::now() - begin;
        printf("\nSaving output at %f seconds...\n", timePassed.count());
        printLock.unlock();

        categoryContainerLock.lock();
        if (params.outputAll) {
            if (allOutputsCategoryContainer.hasBeenUpdated) {
                std::ofstream allResultsFile(params.allOutputFile.c_str());
                allResultsFile << "x = 0, y = 0, rule = B3/S23\n";
                allResultsFile << allOutputsCategoryContainer.CategoriesRLE();
                allResultsFile.close();
                allOutputsCategoryContainer.hasBeenUpdated = false;
            }
        }
        if (oscillatorCategoryContainer.hasBeenUpdated) {
            std::ofstream oscResultsFile(params.oscOutputFile.c_str());
            oscResultsFile << "x = 0, y = 0, rule = B3/S23\n";
            oscResultsFile << oscillatorCategoryContainer.CategoriesRLE();
            oscResultsFile.close();
            oscillatorCategoryContainer.hasBeenUpdated = false;
        }
        if (params.findPartials) {
            if (categoryContainer.hasBeenUpdated) {
                std::ofstream resultsFile(params.outputFile.c_str());
                resultsFile << "x = 0, y = 0, rule = B3/S23\n";
                resultsFile << categoryContainer.CategoriesRLE();
                resultsFile.close();
                categoryContainer.hasBeenUpdated = false;
            }
        }
        categoryContainerLock.unlock();

        printLock.lock();
        std::chrono::time_point<std::chrono::steady_clock> currentTimePoint = std::chrono::steady_clock::now();
        std::chrono::duration<double> currentTime = currentTimePoint - begin;
        std::chrono::duration<double> currentTimeInSearch = currentTimePoint - lastSearchStartTime;
        printf("Output saved, took %f seconds:\n  Total Iterations: %llu\n  Iterations per second: %f\n  First iterations left: %d/%d\n  Estimated time left: %f seconds\n  Stack size: %d\n  Solution types found: %d/%d\n  Oscillating solution types found: %d/%d\n",
            currentTime.count() - timePassed.count(),
            iterations,
            iterations / currentTime.count(),
            firstCatalystsLeft, startingFirstCatalysts,
            currentTimeInSearch.count() * firstCatalystsLeft / (startingFirstCatalysts - firstCatalystsLeft),
            (int)searchDataStack.size(),
            (int)categoryContainer.keys.size(),
            categoryContainer.size,
            (int)oscillatorCategoryContainer.keys.size(),
            oscillatorCategoryContainer.size
            );
        if (params.outputAll) {
            printf("  All solution types found: %d/%d\n", (int)allOutputsCategoryContainer.keys.size(), allOutputsCategoryContainer.size);
        }
        printf("  Internal time: %f seconds\n  Part 2 time amount: %f\n", iterativeSearchTime, isolatedTime / iterativeSearchTime);
        if (params.patternRand) {
            printf("  Tested states size: %d\n  Searches queue/set sizes: %d/%d\n", (int)alreadyTestedLifeStates.size(), (int)newSearchStatesQueue.size(), (int)newSearchStatesSet.size());
        }
        printf("\n");
        printLock.unlock();
    }
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./LocalForce <in file>" << std::endl;
        exit(0);
    }

    Searcher searcher;
    searcher.begin = std::chrono::steady_clock::now();
    
    searcher.Init(argv[1]);

    std::chrono::duration<double> timePassed = std::chrono::steady_clock::now() - searcher.begin;
    printf("Initialization time: %f seconds\n", timePassed.count());

    searcher.Run();

    timePassed = std::chrono::steady_clock::now() - searcher.begin;
    printf("\nSearch Ended\n");
    printf("Total elapsed time: %f seconds\n", timePassed.count());
}
