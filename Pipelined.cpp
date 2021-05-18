#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

// global variables
int pc = 0;
int total_clock_cycles = 0;
int registerfile[31];
int d_mem[31];
int stall = 2;
int once = 0;

struct pipelined{

    // control unit
    int RegWrite = 0;
    int RegDst = 0;
    int Branch = 0;
    int ALUSrc = 0;
    int InstType = 0;
    int MemWrite = 0;
    int MemtoReg = 0;
    int MemRead = 0;
    int Jump = 0;

    //if_id
    int jump_target = 0;
    int alu_zero = 0;
    int branch_target = 0;
    int next_pc;
    string str;

    //id_ex
    int rs = 0;
    int rt = 0;
    int rd = 0;
    int sham = 0;
    string funct;
    string immediate;
    string alu_op;
    int address = 0;

    //ex_mem
    int value = 0;

    //mem_wb
    int result = 0;
    int r = 0;
    int done = 0;

    //controler unit
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
} if_id,id_ex,ex_mem,mem_wb, wb;

// 5 stage   F D E M W
int nop[] = {0,1,1,1,1};

// Function header
void Fetch(string);
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
    
    //run loop for clock cycle
    while(1){
        cout << "total_clock_cycles " << dec << ++total_clock_cycles << ":" << endl;
        //---------------------------Writeback---------------------------
        if(nop[4] == 0){
            //cout << "W\n";
            if (wb.MemWrite == 1){ //sw
                Writeback(wb.value, wb.result);
            }else if (wb.MemRead == 1){ //lw
                Writeback(wb.r, wb.result);
            }else{
                //for I/R
                Writeback(wb.r, wb.value);
            }
            
            //close Writeback
            nop[4] = 1;
            
            if(wb.alu_zero == 1 && wb.Branch == 1){
                nop[3] = 1;
                nop[2] = 1;
                nop[1] = 1;
            }

            //break when no operation in stages
            if(nop[0] == 1 && nop[1] == 1 && nop[2] == 1 && nop[3] == 1 && nop[4] == 1){
                break;
            }

        }
        //------------------------------Mem------------------------------
        if(nop[3] == 0){
            //cout << "M\n";
            // check if its lw, sw, R/I type
            if (mem_wb.MemWrite == 1 || mem_wb.MemRead == 1){ //lw and sw
                Mem(mem_wb.rt, mem_wb.value);
            }else{ //all R/I format
                Mem(mem_wb.rd, mem_wb.value);
            }

            //check harzard
            if(mem_wb.alu_zero == 1 && mem_wb.Branch == 1){
                int flush = BinaryToDec(mem_wb.immediate);
                cout << "control hazard detected (flush " << flush << " instructions)" << endl;
                nop[2] = 1;
                nop[1] = 1;
            }
            
            //send to Writeback
            wb = mem_wb;
            nop[4] = 0;

            //close Mem
            nop[3] = 1;
        }
        //----------------------------Execute----------------------------
        if(nop[2] == 0){
            //cout << "E\n";
            Execute(ex_mem.alu_op, ex_mem.rs, ex_mem.rt, ex_mem.rd, ex_mem.sham, ex_mem.address);

            //make sure the beq runs once
            if(ex_mem.Branch == 1 && ex_mem.alu_zero == 1){
                once = once +1;
            }

            //send to Mem
            mem_wb = ex_mem;
            nop[3] = 0;

            //close Execute
            nop[2] = 1;
        }
        //-----------------------------Decode----------------------------
        if(nop[1] == 0 && stall > 1){
            //cout << "D\n";
            Decode(id_ex.str);
            
            //check data hazard
            //R to R
            if(id_ex.RegDst == 1 && id_ex.RegWrite == 1){
                if(ex_mem.RegDst == 1 && ex_mem.RegWrite == 1){
                    if(id_ex.rs == ex_mem.rd || id_ex.rt == ex_mem.rd){
                            cout << "data hazard detection\n";
                            stall = 0;
                    }
                }
            }
            //R to I(beq)
            if(id_ex.Branch == 1){
                if(ex_mem.RegDst == 1 && ex_mem.RegWrite == 1){
                    if(id_ex.rs == ex_mem.rd || id_ex.rt == ex_mem.rd){
                        cout << "data hazard detection\n";
                        stall = 0;
                    }
                }
            }

            //send to Execute
            ex_mem = id_ex;
            if(stall == 0){
                nop[2] = 1;
            }else{
                nop[2] = 0;
            }

            //close Decode
            nop[1] = 1;
        
        }else{
            if(stall < 2){
                stall++;
                cout << "data hazard detection (nop" << stall << ")\n";
                if(stall == 2){
                    nop[2] = 0;
                }
            }
        }
        //-----------------------------Fetch-----------------------------
        if(nop[0] == 0 && stall > 1){
            stall = 2;
            // wont run fetch when reach last mip instruction
            if(pc/4 == instruction.size()){
                nop[0] = 1;
            }else{
                //cout << "F\n";
                Fetch(instruction[pc/4]);

                //send to decode
                id_ex = if_id;
                nop[1] = 0;
            }
        }
        cout << endl;
    }
    cout << endl;
    cout << "program terminated:\n";
    cout << "total execution time is " << dec << total_clock_cycles << " cycles\n";
}

void Fetch(string instruction){
        //Decode(instruction[pc / 4]);
        if_id.str = instruction;
        // pc update: 
        int next_pc = pc + 4;
        if_id.next_pc = next_pc;
        if (wb.alu_zero == 1 && wb.Branch == 1 && once == 1){
            pc = wb.next_pc + wb.branch_target;
            once -= 1;
        }else if(wb.Jump == 1){
            pc = wb.jump_target;
        }else{
            pc = next_pc;
        }
        cout << "pc is modified to 0x" << hex << pc << endl;

}

void Decode(string str){
    //opcode into Control variables
    string opcode = str.substr(0, 6);
    id_ex.ControlUnit(opcode);

    if (id_ex.Jump == 1){
        // Count the last 26 bit and make pc equal to next jump to instruction
        id_ex.immediate = id_ex.str.substr(6, 26);
        id_ex.address = checkImmediate(id_ex.immediate);
        id_ex.address = id_ex.address << 2; //shift left two (mulitply by 4)
        id_ex.jump_target = id_ex.address;
    }else if (id_ex.Branch == 1){
        // Count rs, rt, alu_op: subtraction, immediate to address to which branch
        id_ex.rs = BinaryToDec(id_ex.str.substr(6, 5));
        id_ex.rt = BinaryToDec(id_ex.str.substr(11, 5));
        id_ex.alu_op = alu(opcode);
        id_ex.immediate = id_ex.str.substr(16, 16);
        id_ex.address = checkImmediate(id_ex.immediate);
        id_ex.branch_target = id_ex.address << 2; //shift left two (mulitply by 4)
    }else{
        // R and I type, excluding beq
        id_ex.rs = BinaryToDec(id_ex.str.substr(6, 5));
        // check it is use rd or rt to store
        if (id_ex.RegDst == 1 && id_ex.RegWrite == 1){
            id_ex.rd = BinaryToDec(id_ex.str.substr(16, 5)); // R: rd =
        }else{
            //lw,sw
            id_ex.rt = BinaryToDec(id_ex.str.substr(11, 5)); // I: rt =
        }

        // ALUSrc controls the shift immediate
        if (id_ex.ALUSrc == 0){ // R
            id_ex.rt = BinaryToDec(id_ex.str.substr(11, 5));
            id_ex.sham = BinaryToDec(id_ex.str.substr(21, 5));
            id_ex.funct = str.substr(26, 6);
            id_ex.alu_op = alu(id_ex.funct); //alu_op is determine by funct
        }else{ 
            // I: lw,sw
            id_ex.immediate = id_ex.str.substr(16, 16);
            id_ex.alu_op = alu(opcode); //alu_op is determine by opcode
            id_ex.address = checkImmediate(id_ex.immediate);
        }
    }
    //cout << "rs: " << id_ex.rs << endl;
    //cout << "rt: " << id_ex.rt << endl;
    //cout << "rd: " << id_ex.rd << endl;
    //cout << "addy: " << id_ex.address << endl;
    //cout << sham << endl;

    //Execute(alu_op, rs, rt, rd, sham, address);
};

void Execute(string alu_op, int rs, int rt, int rd, int sham, int address)
{
    //AND
    if (alu_op == "0000"){ ex_mem.value = registerfile[rs] & registerfile[rt]; }
    //OR
    if (alu_op == "0001"){ ex_mem.value = registerfile[rs] | registerfile[rt]; }
    //subtract, beq
    if (alu_op == "0110"){ ex_mem.value = registerfile[rs] - registerfile[rt]; }
    //NOR
    if (alu_op == "1100"){ ex_mem.value = ~(registerfile[rs] | registerfile[rt]); }
    //lw, sw else add
    if (alu_op == "0010"){ 
        if (ex_mem.ALUSrc == 1){
            ex_mem.value = registerfile[rs] + address;
        }else{
            ex_mem.value = registerfile[rs] + registerfile[rt];
        }
    }
    //set-on-less-than
    if (alu_op == "0111"){ 
        if (registerfile[rs] < registerfile[rt]){
            ex_mem.value = 1;
        }else{
            ex_mem.value = 0;
        }
    }
    //cout << "value: " << ex_mem.value << endl;
    //beq check with alu_zero: 1 means to jump to that branch
    if (ex_mem.value == 0 && ex_mem.Branch == 1){
        ex_mem.alu_zero = 1;
    }else{
        ex_mem.alu_zero = 0;
    }
};

void Mem(int reg, int value){
    if (mem_wb.MemWrite == 1){ //sw
        mem_wb.result = registerfile[reg];
        mem_wb.r = value;
        d_mem[reg] = value;
        cout << "memory 0x" << hex << value << " is modified to 0x" << hex << mem_wb.result << endl;
        //Writeback(address, result);
    }else if (mem_wb.MemRead == 1){ //lw
        // divde address by 4 to place it in the memory by 4
        mem_wb.result = d_mem[value / 4];
        mem_wb.r = reg;
        //Writeback(r, result);
    }else{
        //for I/R
        mem_wb.r = reg;
        mem_wb.result = value;
        //Writeback(r,address);
    }
    //cout << "result: " << mem_wb.result << endl;
    //cout << "r: " << mem_wb.r << endl;
};

void Writeback(int r, int result){
    //cout << "result: " << mem_wb.result << endl;
    //cout << "r: " << mem_wb.r << endl;
    if (wb.RegWrite == 1){ // R & lw
        registerfile[r] = result;
        cout << "$";
        reg(r);
        cout << " is modified to 0x" << hex << result << endl;
    }
};
