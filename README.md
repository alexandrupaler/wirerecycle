# wirerecycle

Prototypical implementation of a quantum circuit optimization method which performs wire recycling. It can optimize
- reversible circuits taken from RevLib, use process.cpp and allcircs.sh from /circs
- ICM circuits obtained from Cuccaro adders use make in root folder

**To execute:**
- make
- ./recycle RAWFileName NrCuccaroQubits ICMDecompose NrOptHeuristic

**The output** of the executable is a single line of the format
TotalQubits AncillaQubits NrRecycledQubits

* RAWFileName - the file obtained after using process.cpp on a RevLib file
* NrCuccaroQubits - the number of qubits the adder can add. Should be >= 4.
* ICMDecompose - possible values 0 (false) or 1 (true) indicating if the input circuit should be ICM decomposed
* NrOptHeuristic - there are two heuristics implemented and possible values are 0 (first heuristic) or 1 (second heuristic)

**Note:** If NrCuccaroQubits is >= 4 the executable will optimize Cuccaro Adders. For optimizing a specified file (as in RawFileName) NrCuccaroQubits should be 0 (zero).
