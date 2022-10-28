# LocalForce
A Game of Life search program based off of CatForce, designed specifically for finding oscillators.

Usage
--
Run `make`, and then `./LocalForce inputfile`. Currently only tested with `g++`.

Input File Format
--
See `input.txt` for an example.

The option delimiter is `" "` - i.e. space. 

| Line                        | Parameter                | Description                                                            |
|-----------------------------|--------------------------|------------------------------------------------------------------------|
| `threads`                   | `n`                      | Number of threads to use                                               |
| `outputFile`                | `filename`               | Output filename                                                        |
| `outputAll`                 | `b`                      | Whether or not to save all valid results                               |
| `findPartials`              | `b`                      | Whether or not to save oscillator partials                             |
|                             | `n`                      | Number of generations after partial period the catalysts must remain   |
| `findShuttles`              | `b`                      | Whether to find partial shuttles                                       |
| `ignoreElaboration`         | `b`                      | Whether to ignore partials with extra catalysts                        |
| `minSaveInterval`           | `n`                      | Interval between saves in seconds                                      |
| `maxOutputsPerRow`          | `n`                      | Maximum number of results to save per row                              |
| `minCatalysts`              | `n`                      | Minimum number of catalysts                                            |
| `maxCatalysts`              | `n`                      | Maximum number of catalysts                                            |
| `maxSlots`                  | `n`                      | Maximum number of catalyst slots                                       |
| `minTransparentCatalysts`   | `n`                      | Minimum number of transparent catalysts                                |
| `maxTransparentCatalysts`   | `n`                      | Maximum number of transparent catalysts                                |
| `minSacrificialCatalysts`   | `n`                      | Minimum number of sacrificial catalysts                                |
| `maxSacrificialCatalysts`   | `n`                      | Maximum number of sacrificial catalysts                                |
| `minInteractionGeneration`  | `n`                      | Minimum interaction generation                                         |
| `maxFirstInteractionGeneration`  | `n`                      | Maximum first interaction generation                                         |
| `maxFirstCatalystInteractionGeneration`  | `n`                      | Maximum first catalyst interaction generation                                         |
| `maxInteractionGeneration`  | `n`                      | Maximum interaction generation                                         |
| `maxGeneration`             | `n`                      | Maximum generation                                                     |
| `outputFile`                | `filename`               | Output filename                                                        |
| `outputFile`                | `filename`               | Output filename                                                        |
| `outputFile`                | `filename`               | Output filename                                                        |
| `outputFile`                | `filename`               | Output filename                                                        |
| `pattern`                   | `rle`                    | The active pattern                                                     |
|                             | `dx dy`                  | Offset                                                                 |
| `symmetry`                  |                          | Base symmetry                                                          |
| `targetSymmetries`          |                          | Symmetries to search                                                   |
| `patternRand`               | `x y`                    | Random pattern search pattern size                                     |
|                             | `(seed)`                 | Optional seed                                                          |
| `avoidRepeatPatternHistories` | `b`                    | Avoid repeating pattern histories in random search (memory-intensive)  |
| `doDelayedSymmetryInteraction` | `b`                   | Whether to allow delayed symmetry interaction                          |
| `cat`                       | `rle`                    | A catalyst                                                             |
|                             | `max-active`             | Number of generations in a row the catalyst may be missing             |
|                             | `dx dy`                  | Offset to centre the catalyst (typically negative)                     |
|                             | `symmetries-char`        | Character denoting the symmetry of the catalyst                        |
|                             | `(slots n)`              | Number of slots the catalyst takes up                                  |
|                             | `(forbidden rle x y)`    | Optional forbidden pattern around the catalyst                         |
|                             | `(anyrequired rle x y)`  | Optional pattern around the catalyst at least one of which is required |
|                             | `(required rle x y)`     | Optional required pattern around the catalyst                          |
|                             | `(antirequired rle x y)` | Optional avoided pattern around the catalyst                           |
|                             | `(transparent)`          | Catalyst is transparent                                                |
|                             | `(sacrificial)`          | Catalyst is sacrificial                                                |
|                             | `(check-recovery)`       | Catalyst checks for recovery when placed                               |
|                             | `(check-recovery-always)` | Catalyst always checks for recovery                                   |

Catalysts use the same format as CatForce, but only stable catalysts are supported.

**Random Patterns**: Using the `patternRand` option will search through random patterns within a certain bounding box. `minInteractionGeneration` should be 0 and `symmetry` should be C1 for random searches.
