#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

// global variables
int pc = 0;
int jump_target = 0;
int alu_zero = 0;
int branch_target = 0;
int total_clock_cycles = 0;
int registerfile[31];
int d_mem[31];

// global control unit
int RegWrite = 0;
int RegDst = 0;
int Branch = 0;
int ALUSrc = 0;
int InstType = 0;
int MemWrite = 0;
int MemtoReg = 0;
int MemRead = 0;
int Jump = 0;

// Function header
void Fetch(vector<string>);
void Decode(string);
void Execute(string, int, int, int, int, int);
void Mem(int, int);
void Writeback(int, int);
void ControlUnit(string);

//calculation/helper
string checkNegative(string immediate)
{
    // loop to switch 1to0 or 0to1
    for (int i = 0; i < immediate.size(); i++)
    {
        if (immediate[i] == '1')
        {
            immediate[i] = '0';
        }
        else
        {
            immediate[i] = '1';
        }
    }
    // loop for adding 1 and break when added
    for (int i = immediate.size() - 1; i > 0; i--)
    {
        if (immediate[i] == '1')
        {
            immediate[i] = '0';
        }
        else
        {
            immediate[i] = '1';
            break;
        }
    }
    return immediate;
}
int checkImmediate(string address){
    if (address[0] == '1')
    {
        address = "-" + checkNegative(address);
    }
    return stoi(address, nullptr, 2);
}
int BinaryToDec(string address)
{
    return stoi(address, nullptr, 2);
}
string alu(string funct)
{
    string output;
    if (funct == "100100")
    { //AND
        output = "0000";
    }
    if (funct == "100101")
    { //OR
        output = "0001";
    }
    if (funct == "100000" || funct == "101011" || funct == "100011") //sw and lw
    {                                                                //add
        output = "0010";
    }
    if (funct == "100010" || funct == "000100")
    { //subtract,BEQ
        output = "0110";
    }
    if (funct == "101010")
    { //set-on-less-than
        output = "0111";
    }
    if (funct == "100111")
    { //NOR
        output = "1100";
    }
    return output;
}
void reg(int temp)
{
    switch (temp)
    {
    case 0:
        printf("zero");
        break;
    case 1:
        printf("at");
        break;
    case 2:
        printf("v0");
        break;
    case 3:
        printf("v1");
        break;
    case 4:
        printf("a0");
        break;
    case 5:
        printf("a1");
        break;
    case 6:
        printf("a2");
        break;
    case 7:
        printf("a3");
        break;
    case 8:
        printf("t0");
        break;
    case 9:
        printf("t1");
        break;
    case 10:
        printf("t2");
        break;
    case 11:
        printf("t3");
        break;
    case 12:
        printf("t4");
        break;
    case 13:
        printf("t5");
        break;
    case 14:
        printf("t6");
        break;
    case 15:
        printf("t7");
        break;
    case 16:
        printf("s0");
        break;
    case 17:
        printf("s1");
        break;
    case 18:
        printf("s2");
        break;
    case 19:
        printf("s3");
        break;
    case 20:
        printf("s4");
        break;
    case 21:
        printf("s5");
        break;
    case 22:
        printf("s6");
        break;
    case 23:
        printf("s7");
        break;
    case 24:
        printf("t8");
        break;
    case 25:
        printf("t9");
        break;
    case 26:
        printf("k0");
        break;
    case 27:
        printf("k1");
        break;
    case 28:
        printf("gp");
        break;
    case 29:
        printf("sp");
        break;
    case 30:
        printf("fp");
        break;
    case 31:
        printf("ra");
        break;
    }
};
int main()
{
    // Initialize: $t1 = 0x20, $t2 = 0x5, $s0 = 0x70
    registerfile[9] = 0x20;
    registerfile[10] = 0x5;
    registerfile[16] = 0x70;
    // Initialize: 0x70 = 0x5, 0x74 = 0x10
    d_mem[28] = 0x5;
    d_mem[29] = 0x10;

    // Input for filename
    cout << "Enter the program file name to run:\n";
    string filename;
    cin >> filename;
    cout << endl;

    // Put mips into a vector and run the single cycle by calling fetch
    ifstream file(filename);
    vector<string> instruction;
    string str;
    while(getline(file, str)){
        instruction.push_back(str);
    }
    Fetch(instruction);
}

void Fetch(vector<string> instruction){
    // while loop from pc 0 --> end of instruction
    while(pc/4 < instruction.size()){
        // Display clock cycle and Decode pc by 4 because address increment by 4
        cout << "total_clock_cycles " << dec << total_clock_cycles+1 << ":" << endl;
        Decode(instruction[pc / 4]);

        // pc update: 
        int next_pc = pc + 4;
        if (alu_zero == 1 && Branch == 1){
            pc = next_pc + branch_target;
        }else if(Jump == 1){
            pc = jump_target;
        }else{
            pc = next_pc;
        }
        cout << "pc is modified to 0x"<< hex << pc <<endl<<endl;
    }
    cout << "program terminated:\n";
    cout << "total execution time is " << total_clock_cycles << " cycles\n";
}

void Decode(string str){
    //opcode into Control variables
    string opcode = str.substr(0, 6);
    ControlUnit(opcode);

    //declare variables
    int rs = 0;
    int rt = 0;
    int rd = 0;
    int sham = 0;
    string funct;
    string immediate;
    string alu_op;
    int address = 0;

    if (Jump == 1){
        // Count the last 26 bit and make pc equal to next jump to instruction
        immediate = str.substr(6, 26);
        address = checkImmediate(immediate);
        address = address << 2; //shift left two (mulitply by 4)
        jump_target = address;
    }else if (Branch == 1){
        // Count rs, rt, alu_op: subtraction, immediate to address to which branch
        rs = BinaryToDec(str.substr(6, 5));
        rt = BinaryToDec(str.substr(11, 5)); 
        alu_op = alu(opcode);
        immediate = str.substr(16, 16);
        address = checkImmediate(immediate);
        branch_target = address << 2; //shift left two (mulitply by 4)
    }else{
        // R and I type, excluding beq
        rs = BinaryToDec(str.substr(6, 5));
        // check it is use rd or rt to store
        if (RegDst == 1 && RegWrite == 1){
            rd = BinaryToDec(str.substr(16, 5)); // R: rd =
        }else{
            //lw,sw
            rt = BinaryToDec(str.substr(11, 5)); // I: rt =
        }

        // ALUSrc controls the shift immediate
        if (ALUSrc == 0){ // R
            rt = BinaryToDec(str.substr(11, 5));
            sham = BinaryToDec(str.substr(21,5));
            funct = str.substr(26, 6);
            alu_op = alu(funct); //alu_op is determine by funct  
        }else{ 
            // I: lw,sw
            immediate = str.substr(16, 16);
            alu_op = alu(opcode); //alu_op is determine by opcode
            address = checkImmediate(immediate);
        }
    }
    //cout << rs << endl;
    //cout << rt << endl;
    //cout << rd << endl;
    //cout << address << endl;
    //cout << sham << endl;

    Execute(alu_op, rs, rt, rd, sham, address);
};

void Execute(string alu_op, int rs, int rt, int rd, int sham, int address)
{
    int value = 0;
    //AND
    if (alu_op == "0000"){ value = registerfile[rs] & registerfile[rt]; }
    //OR
    if (alu_op == "0001"){ value = registerfile[rs] | registerfile[rt]; }
    //subtract, beq
    if (alu_op == "0110"){ value = registerfile[rs] - registerfile[rt]; }
    //NOR
    if (alu_op == "1100"){ value = ~(registerfile[rs] | registerfile[rt]); }
    //lw, sw else add
    if (alu_op == "0010"){ 
        if (ALUSrc == 1){
            value = registerfile[rs] + address;
        }else{
            value = registerfile[rs] + registerfile[rt];
        }
    }
    //set-on-less-than
    if (alu_op == "0111"){ 
        if (registerfile[rs] < registerfile[rt]){
            value = 1;
        }else{
            value = 0;
        }
    }
    //cout << value << endl;
    //beq check with alu_zero: 1 means to jump to that branch
    if (value == 0 && Branch == 1){
        alu_zero = 1;
    }else{
        alu_zero = 0;
    }

    // check if its lw, sw, R/I type
    if (MemWrite == 1 || MemRead == 1){ //lw and sw
        Mem(rt, value);
    }else{ //all R/I format
        Mem(rd, value);
    }
};

void Mem(int r, int address){
    int value = 0;
    if (MemWrite == 1){ //sw
        value = registerfile[r];
        Writeback(address, value);
    }else if (MemRead == 1){ //lw
        // divde address by 4 to place it in the memory by 4
        value = d_mem[address / 4];
        Writeback(r, value);
    }else{
        //for I/R
        Writeback(r,address);
    }
};

void Writeback(int r, int result){
    if (RegWrite == 0 && MemWrite == 1){ //sw
        d_mem[r] = result;
        cout << "memory 0x" << hex << r << " is modified to 0x" << hex << result << endl;
    }else if (RegWrite == 1){ // R & lw
        registerfile[r] = result;
        cout << "$";
        reg(r);
        cout << " is modified to 0x" << hex << result << endl;
    }
    total_clock_cycles = total_clock_cycles + 1;
};

void ControlUnit(string opcode)
{
    if (opcode == "000000") // r format
    {                       // add,sub,and,or,slt,nor
        RegWrite = 1;
        RegDst = 1;
        Branch = 0;
        ALUSrc = 0;
        InstType = 1;
        MemWrite = 0;
        MemtoReg = 0;
        MemRead = 0;
        Jump = 0;
    }
    if (opcode == "100011")
    { // lw
        RegWrite = 1;
        RegDst = 0;
        Branch = 0;
        ALUSrc = 1;
        InstType = 0;
        MemWrite = 0;
        MemtoReg = 1;
        MemRead = 1;
        Jump = 0;
    }
    if (opcode == "101011")
    { // sw
        RegWrite = 0;
        //RegDst = 0;
        Branch = 0;
        ALUSrc = 1;
        InstType = 0;
        MemWrite = 1;
        //MemtoReg = 0;
        MemRead = 0;
        Jump = 0;
    }
    if (opcode == "000100")
    { // beq
        RegWrite = 0;
        //RegDst = 0;
        Branch = 1;
        ALUSrc = 0;
        InstType = 0;
        MemWrite = 0;
        //MemtoReg = 0;
        MemRead = 0;
        Jump = 0;
    }
    if (opcode == "000011" || opcode == "000010") // J format
    {
        RegWrite = 0;
        //RegDst = 0;
        Branch = 0;
        //ALUSrc = 0;
        //InstType = 0;
        MemWrite = 0;
        //MemtoReg = 0;
        MemRead = 0;
        Jump = 1;
    }
};
