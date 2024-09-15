# Logic Function Reducer
*Developed by [@dabecart](https://dabecart.net/)*, 2024.

A really useful tool to reduce logic functions on your digital circuits using Petrick's Method. For large circuits with more than four inputs, using K-maps (Karnaugh Maps) becomes really unpractical! For that, let the heavy work of finding a reduced expression to this program! 

And remember, the more reduced the expression is, the lesser number of logic gets you'll use!

To use the program, first create a truth table similar to [this one](truth_table.txt). The program will get its reduced algebraic expression in *minterms*; that is, a summation of the product of bits or inputs of the logical function. 

You may also specify the *Do-Not-Care* (DNC) bits for the function. These are the group of impossible inputs, and so, as these inputs may never occur, their result on the function may be either 0 or 1 without any care to us. This greatly reduces functions as the algorithm will cleverly choose them to be 0 or 1 so that, at the end, we get the least number of operations for out final result.

## Output example
For the [truth table](truth_table.txt) of seven inputs (a,b,c,d,e,f,g) and four outputs (Q0, Q1, Q2, Q3), from left to right:

```
0 0 0 0 x x x | 0 0 0 1
0 0 0 1 x x x | 0 0 1 1
0 0 1 0 x x x | 0 0 1 0
0 0 1 0 x x x | 0 1 0 0
0 0 1 1 x x x | 0 1 0 1
0 1 0 0 0 x x | 0 0 1 0
0 1 0 0 1 x x | 0 1 1 0
0 1 0 1 x 0 x | 0 1 1 0
0 1 1 0 x 1 x | 0 1 1 1
0 1 1 1 x x x | 1 0 0 0
1 0 0 0 x x 0 | 0 0 0 0
1 0 0 0 x x 1 | 1 0 0 1
1 0 0 1 x x x | 1 0 1 0
1 0 1 0 0 x x | 0 1 1 0
1 0 1 0 1 x x | 1 0 1 1
1 0 1 1 x x x | 0 0 0 1
1 1 x x x x x | x x x x
```

You'll get:

```
$ python3 LogicReducer.py truth_table.txt 

Q0: [56, 57, 58, 59, 60, 61, 62, 63, 65, 67, 69, 71, 72, 73, 74, 75, 76, 77, 78, 79, 84, 85, 86, 87]
DNC0: [96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127]
Q: [acd+bcd+acg+acde]  Number of operations: 15

Q1: [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 36, 37, 38, 39, 40, 41, 44, 45, 50, 51, 54, 55, 80, 81, 82, 83]
DNC1: [96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127]
Q: [bcdf+abc+bcde+bcdf+acde]  Number of operations: 27

Q2: [8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 44, 45, 50, 51, 54, 55, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87]
DNC2: [96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127]
Q: [bcd+bcd+bcf+bdf]  Number of operations: 18

Q3: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31, 50, 51, 54, 55, 65, 67, 69, 71, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95]
DNC3: [96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127]
Q: [ace+bcdf+abc+bcd+acdg]  Number of operations: 23
```

## How to run
You've got two tools and they're both callable by console:

- **LogicReducer**. Is a Python program that converts a truth table stored into a text file into a list of minterms. These are fed to the **Petrick** program so that they get reduced. With this program you will get all the reduced expressions for the columns at the right of the truth table. 
  
  You can read the *help* menu for ussage by calling:
  - Windows:
    ```
    python LogicReducer.py
    ```
  - Linux:
    ```
    $ python3 LogicReducer.py -h
    ```

- **Petrick**. This is a C++ program that takes the number of inputs, the minterms and Do-Not-Care bits and generates the reduced algebraic expression for those inputs. 

  You can read the *help* menu for ussage by calling:
  - Windows:
    ```
    petrick.exe -h
    ```
  - Linux:
    ```
    $ ./petrick -h
    ```

## How to compile

Python files do not need compilation, simply run [LogicReducer.py](LogicReducer.py) with the Python interpreter.

To compile [petrick.cpp](petrick.cpp) use `g++`:

```
$ g++ -o petrick petrick.cpp
```

I recommend Windows users to use [MSYS2](https://www.msys2.org/) for the compilation toolchain. Follow [this guide](https://code.visualstudio.com/docs/cpp/config-mingw) for the installation with VSCode.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.