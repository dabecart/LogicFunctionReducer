/************************************************************************************************//*
* @file petrick.cpp
* @brief Takes the number of inputs, the minterms and Do-Not-Care bits and generates the reduced 
* algebraic expression for those inputs.
*
* @project   Logic Function Reducer
* @version   1.0
* @date      2024-09-15
* @author    @dabecart
*
* @license
* This project is licensed under the MIT License - see the LICENSE file for details.
***************************************************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <initializer_list>
#include <sstream>

using namespace std;

// #define VERBOSE

typedef struct Minterm{
  int val;
  bool dnc; // Do not care minterm
  int bitCount;

  // A minterm represents a combination of bits that produce 1 on the output of the function. 
  Minterm(int x, bool isDNC = false) : val(x), dnc(isDNC){
    this->bitCount = countBits(x);
  }

  Minterm operator^(Minterm other) const{
    return Minterm(this->val^other.val);
  }

  Minterm operator&(Minterm other) const{
    return Minterm(this->val&other.val);
  }

  Minterm operator~(){
    return ~this->val;
  }

  bool operator<(Minterm other){
    return this->val<other.val;
  }
  bool operator>(Minterm other){
    return this->val>other.val;
  }
  bool operator==(Minterm other){
    return this->val==other.val;
  }
  bool operator!=(Minterm other){
    return this->val!=other.val;
  }

  static int countBits(int value){
    int bitCount = 0;
    while(value){
      if(value & 0x01) bitCount++;
      value = static_cast<unsigned int>(value)>>1;
    }
    return bitCount;
  }

}Minterm;
typedef vector<Minterm> Minterms;

typedef struct Implicant {
  private:
  vector<Minterm> mins;
  
  public:
  // The mask is used to 'group' the minterms. The mask is all ones, meaning that this implicant is defined
  // by all the bits of the minterms. Whenever a bit of this mask is zero, it means that said bit is not common
  // to the minterms of this implicant. For example, m(4,12)'s mask is 0111 because bit 3 is not shared between
  // 4 (0100) and 12 (1100).
  Minterm commonBitsMask = -1;
  bool essential = true;
  // This name is used to simplify the output of Petrick Algorithm.
  char name;

  Implicant(){
    mins.clear();
  }

  // This is so a Minterm list can be initialized with a {1,2,3,4} for example
  Implicant(initializer_list<Minterm> initList) {
    mins.clear();
    for(Minterm m : initList){
      this->mins.push_back(m);
    }
  }

  int size(){
    return mins.size();
  }

  /**
   * @brief Joins, if it is possible, two implicants. 
   * 
   * @param m The other implicant to join.
   * @param out The result implicant if it is possible to join.
   * @return true If two implicants can be joined.
   * @return false If opposite.
   */
  bool joinWith(Implicant &m, Implicant &out){
    if(this->commonBitsMask != m.commonBitsMask || this->size() != m.size()) return false;

    Minterm result = (this->mins[0] & commonBitsMask)^(m.mins[0] & commonBitsMask);
    if(result.bitCount != 1) return false;

    // If both minterms that were joined were not essential, then set the new one as no essential.
    if(!this->essential && !m.essential) out.essential = false;
    else out.essential = true;

    // Copy all the minterms of the inputs to the new implicant
    for(int i = 0; i < this->size(); i++){ 
      out.mins.push_back(this->mins[i]);
      out.mins.push_back(m.mins[i]);
    }
    // Sort the minterms
    out.sort();

    // Set the new mask as the combination of the common bit in result and the original mask.
    out.commonBitsMask = this->commonBitsMask&(~result);
    return true;
  }

  // Insertion sort.
  void sort(){
    int n = mins.size();
    for (int i = 1; i < n; ++i) {
      int key = mins[i].val;
      int j = i - 1;
      while (j >= 0 && mins[j].val > key) {
        mins[j + 1] = mins[j];
        j = j - 1;
      }
      mins[j + 1] = key;
    }
  }

  Minterm& operator[](int index){
    return mins[index];
  }

  bool operator==(Implicant imp){
    if(this->size() != imp.size()) return false;
    for(Minterm m1 : this->mins){
      bool foundPair = false;
      for(Minterm m2 : imp.mins){
        foundPair = m1.val == m2.val;
        if(foundPair) break;
      }
      if(!foundPair) return false;
    }
    return true;
  }

  void print(){
    cout << name;
  }

  void printDetailed(){
    cout << name <<  " = m(";
    for(int i = 0; i < mins.size(); i++){
      cout << mins[i].val;
      if(i != mins.size()-1) cout << ",";
    }
    cout << ") Mask: " << (~commonBitsMask).val;
    if(this->essential){
      cout << " Essential";
    }
  }

  void printAlgebraic(int functionBitSize){
    char out = 'a';
    functionBitSize--;
    for(;functionBitSize >= 0; functionBitSize--){
      if((commonBitsMask.val>>functionBitSize)&0x01){
        if((mins[0].val>>functionBitSize)&0x01){
          cout << "\e[0;32m";
          cout << out;
          cout << "\e[0m";
        }else{
          cout << "\e[0;31m";
          cout << out;
          cout << "\e[0m";
        }
      }
      out++;
    }
  }

  // Number of logic gates or operations that are needed to define this implicant.
  int getOperationCount(int functionBitSize){
    // Start on -1 as if there are three members being multiplied, we will do two multiplications.
    int opCount = -1;
    for(int i = 0; i < functionBitSize; i++){
      // Only the bits that are common are used to calculate the number of operations.
      if((commonBitsMask.val>>i)&0x01){
        opCount++; // Multiplication gate.
        // If the value is negated, we need to add a NOT gate.
        if(!((mins[0].val>>i)&0x01)){
          opCount++;
        }
      }
    }
    return opCount;
  }

}Implicant;

typedef vector<Implicant> Implicants;

typedef enum OperationType{
  IMPLICANT_SUM,
  IMPLICANT_MULT,
} OperationType;

/**
 * @brief An ImplicantOperation represents a sum or multiplication of implicants.
 */
typedef struct ImplicantOperation{
  // If this is a simple ImplicantOperation, it will reference to a single implicants object. 
  Implicant* imp = 0;

  // Stores all the implicants (stored as ImplicantOperation-s) that are being multiplied.
  vector<ImplicantOperation> operators;
  // Type of operation. By default it will be a multiplication.
  OperationType type = IMPLICANT_MULT;

  ImplicantOperation() {}
  ImplicantOperation(Implicant* imps) : imp(imps){}

private:
  void levelParenthesis(vector<ImplicantOperation> &previousList, OperationType operationLevel){
    if(type != operationLevel || imp!=0){
      previousList.push_back(*this);
    }else{
      for(ImplicantOperation i : operators){
          i.levelParenthesis(previousList, operationLevel);
      }
    }
  }

public:
  // Puts the function on the same level of parenthesis.
  // [m(0,1)+[m(0,1)*m(1,5)]]+[[m(0,2)*m(0,1)]+[m(0,2)*m(1,5)]] => [ m(0,1) + [m(0,1)*m(1,5)] + [m(0,2)*m(0,1)] + [m(0,2)*m(1,5)] ]
  void levelParenthesis(){
    vector<ImplicantOperation> tempList;
    levelParenthesis(tempList, type);
    operators = tempList;
  }

  /**
   * @brief Applies A + A*B = A. Supposes that the input is a minterm, meaning a sum of multiplications.
   * 
   * @return true If any change happened.
   */
  bool applySumAbsortion(){
    if(type != IMPLICANT_SUM) return false;

    bool anyChange = false;
    for(auto it = operators.begin(); it != operators.end(); it++){
      ImplicantOperation op1 = *it;
      for(auto start = it+1; start!=operators.end();){
        ImplicantOperation op2 = *start;
        // Search op1 inside op2.
        if(op2.searchImplicant(op1)){
          operators.erase(start);
          // anyChange = true; // No need to check again, as the object to the right is the one being removed and it does not need to be later checked.
        // Search op2 inside op1.
        }else if(op1.searchImplicant(op2)){
          // If it is found, we switch the implicants so op1 is now op2.
          // Suppose AB, B, A, AC, C. When comparing AB(op1) with A(op2), A is in AB, so switch the values and erase the latter one.
          // As A is greater in scope than AB, I should group all AB groups and more. 
          iter_swap(it,start);
          operators.erase(start);
          anyChange = true;
        }else{
          start++;
        }
      }
    }
    return anyChange;
  }

  ImplicantOperation operator+(ImplicantOperation other){
    // This is an empty variable.
    if(imp==0 && operators.size()==0) return other;

    // cout << "\e[0;34m";
    // this->print();
    // cout << "+";
    // other.print();
    // cout << "\e[0m" << endl;

    // Fastly apply X + X = X.
    if(*this == other) return other;

    // Normal sum (do not apply distributive property).
    ImplicantOperation ret;
    ret.type = IMPLICANT_SUM;
    ret.operators.push_back(*this);
    ret.operators.push_back(other);
    return ret;
  }

  ImplicantOperation operator*(ImplicantOperation other){
    // This is an empty variable.
    if(imp==0 && operators.size()==0) return other;

    // cout << "\e[0;34m";
    // this->print();
    // cout << "*";
    // other.print();
    // cout << "\e[0m" << endl;

    // Fastly apply X * X = X.
    if(*this == other) return other;
    
    // Distributive property X * (X + Y) = XX + XY = X + XY
    ImplicantOperation a = *this, b = other;
    if(a.type == IMPLICANT_SUM){
      ImplicantOperation sum;
      for(ImplicantOperation op : a.operators){
        sum = sum + (b*op); // Recursive
      }
      return sum;
    }
    if(b.type == IMPLICANT_SUM){
      ImplicantOperation sum;
      for(ImplicantOperation op : b.operators){
        sum = sum + (a*op); // Recursive
      }
      return sum;
    }

    // Apply idempotent law X * XY = XY
    if(a.searchImplicant(b)){
      return a;
    }
    if(b.searchImplicant(a)){
      return b;
    }

    ImplicantOperation ret;
    ret.type = IMPLICANT_MULT;
    ret.operators.push_back(*this);
    ret.operators.push_back(other);
    ret.levelParenthesis();
    return ret;
  }

  /**
   * @brief Searches for other inside this ImplicantOperation.
   * 
   * @param other The ImplicantOperation to look for.
   * @return true if other is contained inside this. false otherwise.
   */
  bool searchImplicant(ImplicantOperation other){
    // Must be same type (+/*) of operation. Other cannot have a number of implicants greater than this (ABC in A?, of course not)
    if(this->type!=other.type || other.operators.size() > this->operators.size()) return false;

    // If they are single minterms. 
    if(this->imp!=0 && other.imp!=0){
      return this->imp==other.imp;
    }

    // If it is a minterm...
    if(other.imp){
      for(ImplicantOperation imp2 : operators){
        if(other.imp == imp2.imp) return true;
      }
      return false;
    }else{
      for(ImplicantOperation imp1 : other.operators){
        bool found = false;
        for(ImplicantOperation imp2 : operators){
          if(imp1 == imp2){ // Recursion.
            found = true;
            break;
          }
        }
        if(!found) return false;
      }
    }
    return true;
  }

  bool operator==(ImplicantOperation other){
    // Must be same type (+/*) of operation and same size.
    if(this->type!=other.type || this->operators.size()!=other.operators.size()) return false;

    // If they are single minterms. 
    if(this->imp!=0 && other.imp!=0){
      return this->imp==other.imp;
    }

    if(operators.size() != other.operators.size()) return false;

    for(ImplicantOperation imp1 : operators){
      bool found = false;
      for(ImplicantOperation imp2 : other.operators){
        if(imp1 == imp2){ // Recursion.
          found = true;
          break;
        }
      }
      if(!found) return false;
    }
    return true;
  }

  bool operator!=(ImplicantOperation other){
    return !((*this)==other);
  }

  int getOperationCount(int functionBitSize){
    if(imp == 0){
      int opers = operators.size() - 1; // Number of OR operations.
      for(ImplicantOperation ops : operators){
        opers += ops.getOperationCount(functionBitSize); // Number of AND and NOT operations.
      }
      return opers;
    }else{
      return imp->getOperationCount(functionBitSize); // Number of AND and NOT operations.
    }
  }

  void print(){
  #ifdef VERBOSE
    if(imp){
      imp->print();
    }else{
      cout << "[";
      for(int i = 0; i < operators.size(); i++){
        operators[i].print();
        if(i != operators.size()-1){
          if(type == IMPLICANT_SUM) cout << "+";
          else if(type == IMPLICANT_MULT) cout << "*";
        }
      }
      cout << "]";
    }
  #endif
  }

  void printAlgebraic(int functionBitSize){
    if(imp){
      imp->printAlgebraic(functionBitSize);
    }else{
      cout << "[";
      for(int i = 0; i < operators.size(); i++){
        operators[i].printAlgebraic(functionBitSize);
        if(i != operators.size()-1){
          // When passing from an ImplicantOperation to minterms, the operations are reversed.
          // Normally one single ImplicantOperation is to be output.
          if(type == IMPLICANT_SUM) cout << "*";
          else if(type == IMPLICANT_MULT) cout << "+";
        }
      }
      cout << "]";
    }
  }
}ImplicantOperation;

typedef struct Function{
  Implicants originalFunction;
  Implicants imps;
  // Number of inputs
  int numInputs;
  string funcName;
  
  Function(Minterms m, Minterms dnc, int nInp, string name) : numInputs(nInp), funcName(name){
    // Put both minterms inside the implicant function as separate implicants but in order.
    int mIndex = 0, dIndex = 0;
    while(mIndex != m.size() || dIndex != dnc.size()){
      Implicant tempImp;
      if(mIndex == m.size()){
        dnc[dIndex].dnc = true;
        tempImp = {dnc[dIndex++]};
        tempImp.essential = false;
      }else if(dIndex == dnc.size()){
        tempImp = {m[mIndex++]};
      }else if(m[mIndex] < dnc[dIndex]){
        tempImp = {m[mIndex++]};
      }else if(m[mIndex] > dnc[dIndex]){
        dnc[dIndex].dnc = true;
        tempImp = {dnc[dIndex++]};
        tempImp.essential = false;
      }else{
        throw invalid_argument("Input of two minterms as Do not care and Do care");
      }
      originalFunction.push_back(tempImp);
    }
  }

  void reduce(){
    calculateImplicants();
    removeNonEssentialImplicants();
    #ifdef VERBOSE
      nameImplicants();
    #endif
    petrick();
  }

  void printTruthTable(){
      for(int i = 0; i < numInputs; i++){
          cout << (char)('a'+i);
      }
      cout << "  " << funcName << endl;

      for(int i = 0; i < (1<<numInputs); i++){
          for(int j = numInputs-1; j >= 0; j--){
              if(i&(1<<j)){
                  cout << '1';
              }else{
                  cout << '0';
              }
          }

          cout << "  ";
          int search = searchMinterm(i);
          if(search == 0){
              cout << '1';
          }else if(search == 1){
              cout << 'x';
          }else{
              cout << '0';
          }
          cout << endl;
      }
  }

  private:
  void petrick(){
    // Convert implicants to operations.
    ImplicantOperation ops[imps.size()];
    for(int i = 0; i < imps.size(); i++){
      ImplicantOperation op(&imps[i]);
      ops[i] = op;
    }

    ImplicantOperation result;
    bool resultValueSet = false;
    // From the prime implicant chart, we shall group the implicants that share the same minterm value.
    ImplicantOperation mult;
    for(Implicant i : originalFunction){
      Minterm min = i[0];
      if(min.dnc) continue;  // If it is a DNC, no need to add it.

      ImplicantOperation sum;
      // All implicants...
      for(ImplicantOperation op : ops){
        Implicant imp = *op.imp;
        // Minterms of this implicant...
        for(int i = 0; i < imp.size(); i++){
          // If the implicant contains the minterm.
          if(imp[i] == min){
            sum = sum + op;
            break;
          }
        }
      }
      sum.print();
      mult = mult * sum;
#ifdef VERBOSE
      cout<<endl;
      mult.print();
      cout<<endl<<"****************"<<endl;
#endif      
      mult.levelParenthesis();
      // Simplify till no changes are made.
      while(mult.applySumAbsortion()){}
    }

#ifdef VERBOSE
    mult.print();
    cout << "   SIZE:" << mult.operators.size() << endl;
#endif

    cout << this->funcName << ": ";
    // Select the term with the implicant with the least minterms if it is a sum.
    int leastOperationCount;
    if(mult.type == IMPLICANT_SUM){
      leastOperationCount = mult.operators[0].getOperationCount(numInputs);
      int leastOperationIndex = 0;
      for(int i = 1; i < mult.operators.size(); i++){
        int thisOpCount = mult.operators[i].getOperationCount(numInputs); 
        if(thisOpCount < leastOperationCount){
          leastOperationCount = thisOpCount;
          leastOperationIndex = i;
        }
      }
      mult.operators[leastOperationIndex].printAlgebraic(numInputs);
    }else{
      leastOperationCount = mult.getOperationCount(numInputs);
      mult.printAlgebraic(numInputs);
    }

    cout << "  Number of operations: " << leastOperationCount << endl;
  }

  void calculateImplicants(){
    // Copy the minterms to the implicants.
    for(int i = 0; i < originalFunction.size(); i++){
      Implicant copy({originalFunction[i]});
      imps.push_back(copy);
    }

    // Group the implicants. Max implicant group has the size of the number of bits (inputs of function).
    int previousImplicantsAddedCount = imps.size();
    for(int impSize = 0; impSize < numInputs; impSize++){
      // Stores the indeces of the implicants that have been combined so that after this iteration, they are marked as no essential
      // as they have been 'reduced' to other implicant.
      vector<int> impIndexCombined;
      // Number of new implicants in this loop iteration. This is a simple optimization to reduce steps in the inner loop.
      int newImplicantsCount = 0;
      // Search for a pair of compatible implicants, starting from the last implicant added on the last iteration of this loop.
      for(int i = imps.size()-previousImplicantsAddedCount; i < imps.size(); i++){
        for(int j = i+1; j < imps.size()-newImplicantsCount; j++){
          // imps[i].print();
          // cout << " ";
          // imps[j].print();
          // cout << endl;

          Implicant newImp;
          if(!imps[i].joinWith(imps[j], newImp)) continue;

          // If the originals can be combined, then they were not essentials.
          impIndexCombined.push_back(i);
          impIndexCombined.push_back(j);
          
          if(implicantListContains(newImp)) continue;

          imps.push_back(newImp);
          newImplicantsCount++;
        }
      }

      // Mark reduced implicants as non essential.
      for(int i : impIndexCombined){
        imps[i].essential = false;
      }

      previousImplicantsAddedCount = newImplicantsCount;
    }
  }

  void removeNonEssentialImplicants(){
    for(auto it = imps.begin(); it != imps.end();){
      Implicant imp = *it;
      if(!imp.essential) it = imps.erase(it);
      else it++;
    }

    if(imps.size() == 0){
      throw runtime_error("This function does not have essential implicants (wut?)");      
    }
  }

  bool implicantListContains(Implicant i){
    for(Implicant listImp : imps){
      if(listImp == i) return true;
    }
    return false;
  }

  void nameImplicants(){
    char letter = 'A';
    for(int i = 0; i < imps.size(); i++){
      imps[i].name = letter++;
      imps[i].printDetailed();
      cout << endl;
    }
  }

  // @return 0 if it is a minterm, 1 if it is a 'do not care' bit, and -1 if it has not been found.
  int searchMinterm(int n){
    for(int i = 0; i < originalFunction.size(); i++){
      Minterm x = originalFunction[i][0];
      if(x.val == n) return x.dnc;
    }
    return -1;
  }
}Function;

// Function to display the help menu
void displayHelp() {
    cout << "Usage: ./petrick <numInputs> [<minterms>] [<dncs>]\n";
    cout << "Example: ./program 3 [1,2,3] [4,5,6]\n\n";
    cout << "Arguments:\n";
    cout << "<numInputs>  : The number of inputs of the logic function.\n";
    cout << "[<minterms>] : The minterms of the function. Must be a comma-separated list of\n";
    cout << "               numbers enclosed in [].\n";
    cout << "[<dncs>]     : The Do-Not-Care terms of the function. Must be a comma-separated \n";
    cout << "               list of numbers enclosed in [].\n";
}

// Function to parse an array from a string (e.g., "[1,2,3]")
Minterms parseArrayToMinterms(const string& arrayStr) {
    Minterms result;
    
    // Ensure the input string starts with '[' and ends with ']'
    if (arrayStr.front() != '[' || arrayStr.back() != ']') {
        cerr << "Error: Array should be enclosed in [].\n";
        return result;
    }
    
    // Remove the brackets
    string innerStr = arrayStr.substr(1, arrayStr.size() - 2);
    
    // Split the string by commas
    stringstream ss(innerStr);
    string num;
    
    while (getline(ss, num, ',')) {
        result.push_back(Minterm(stoi(num)));
    }
    
    return result;
}

int main(int argc, char* argv[]){
    // Check if help is requested.
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        displayHelp();
        return 0;
    }

    if (argc != 4) {
        std::cerr << "Error: Invalid number of arguments.\n";
        displayHelp();
        return 1;
    }
    // Parse the first argument.
    int numberOfInputs;
    try {
        numberOfInputs = stoi(argv[1]); 
    } catch (...) {
        std::cerr << "Error: The first argument must be a valid number.\n";
        return 1;
    }

    // Parse the second and third arguments as arrays.
    Minterms minterms = parseArrayToMinterms(argv[2]);
    Minterms dnc = parseArrayToMinterms(argv[3]);

    // Generate function, reduce and print the results.
    Function func = Function(minterms, dnc, numberOfInputs, "Q");
    func.reduce();

}