

## LR1 Parser


### Building and Executing

Use the following commands to build the code in module-1 (assuming the current directory contains the src folder). After successfully running these commands, _<span style="text-decoration:underline;">./build/Module_1/main.out</span>_ will be generated, and it can be executed to process the given CFGs to generate the LR1 parse table and to parse inputs. On systems with the Bash shell, use _<span style="text-decoration:underline;">./buildModule1.sh</span>_ to build the program and _<span style="text-decoration:underline;">./testM1.sh</span>_ to compile and execute the program with _<span style="text-decoration:underline;">./grammar_2/inp.txt</span>_ as an input. Do note that ./testM1.sh redirects output to _<span style="text-decoration:underline;">./grammar_2/out.txt</span>_.


```
mkdir -p build
mkdir -p build/Module_1
###
flex -o ./src/Module_1/lex.yy.c ./src/Module_1/lex.l
gcc -c -o ./build/Module_1/lex.out ./src/Module_1/lex.yy.c
###
g++ -c -o ./build/Module_1/LR1.out ./src/Module_1/LR1.cpp
g++ -c -o ./build/Module_1/SetOfItems.out ./src/Module_1/SetOfItems.cpp
g++ -c -o ./build/Module_1/mainTmp.out ./src/Module_1/main.cpp
g++ -o  ./build/Module_1/main.out \
       ./build/Module_1/LR1.out \
       ./build/Module_1/SetOfItems.out \
       ./build/Module_1/lex.out \
       ./build/Module_1/mainTmp.out
```



## How to use?



* Examples:
    * Check the folders ./grammar_1 and ./grammar_2 for examples (Both grammars are taken from Lab Assignment 5)
    * The _<span style="text-decoration:underline;">inp.txt</span>_ file is used as input and the generated output is stored in _<span style="text-decoration:underline;">out.txt</span>_. (grammar_2’s parsing status starts from line number 4252)
    * The _<span style="text-decoration:underline;">LR1_Grammar.txt</span>_ files in both the folders contain the information required to build an LR(1) parse table.
* Input:
    * First CFG related details would be asked by the program (non-terminals, terminals, start symbol, and production rules)
    * After that, it’ll prompt you to enter a directory name. The relative/absolute path of any existing directory can be entered. If it is left empty, it’ll use the current directory.
    * It’ll then create the “LR1_Grammar.txt” file in the directory. This file contains information about the grammar and LR1 parse table. (i.e. GOTO and REDUCTION for all the states of the grammar)
    * To verify the correctness of the parse table, the program will then prompt for test input. In case the input is provided via command line, ^D (ctrl+d or equivalent EOF command for the current OS) should be used to indicate the end of the input.
    * After the end-of-input, the program will **show each parsing step (with states and symbols)**, and then it 2 will provide a verdict.
    * **NOTE:** for large grammars, like the grammar_2 of the LL1 assignment (Lab Assignment 5), the output is very long. So, it is advisable to redirect the output to some file.


## Implementation Details



* Object-oriented code (SetOfItems and LR1 classes)
* A lot of intermediate/derived information (most of the time stored as an unordered_map/HashMap mapping a key/id to the corresponding instance pointer) that provides efficiency
* While we are creating a new LR(1) DFA state, we must check if that state already exists. To implement this, the program computes a **unique hash for each DFA state**. It can be observed that the closures of the set of the same items are the same, and it can also be observed that the way we construct LR1 states, the same closure set can only be derived by the same set of items. So, we should compute hashes before computing the closure of a set. 
    * Hash of a state contains hashes of all its items seperated by ‘|’ in sorted order. (Sorted order is important as it ensures that ordering of items within the set will not affect the hash value)
    * In the program, each production rule and symbol is assigned an integer id. 
    * Hash of an item is in the format: “[ production-rule-id # dot-index # space separated sorted ids of lookup symbols ]”.
        * This format ensures that two items will have the same iff they have the same production rule, dot position, and lookup symbols.
* The program is optimized by passing around and copying the pointers/references instead of the full objects. Destructors ensure that there are no memory leaks. (On my local machine, the program takes less than 50ms to build LR(1) DFA, to create the parse table, and to test the input for grammar_2, which has 258 states and 1614 entries in the parse table)