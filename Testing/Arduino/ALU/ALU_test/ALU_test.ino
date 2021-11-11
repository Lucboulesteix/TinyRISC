int Alow = 2;
int Ahigh = 17;
int Blow = 18;
int Bhigh = 33;
int Outlow = 34;
int Outhigh = 49;
int ALUlow = Outhigh + 50;
int ALUhigh = ALUlow + 53;


int delaytimer = 20;
unsigned short Rd;
int wordsize = 16;
int cmdsize = 4;
char* Rsptr;
char* Roptr;
char* Rdptr;
char* cmdptr;
char* ALUCOMM;
char* optype;

int bintoDec(char* binary, int blength){
  int sum = 0;
  for ( int i = 0 ; i < blength; i++){
    sum = sum + binary[i]*(pow(2,(blength-1-i)));
  }
  return sum;
}

bool chartoBool(char s) { //converts a char (ASCII) to an equivalent boolean (HIGH/LOW) value
  bool q;
  if (s == '0') {
    q = LOW;
  }
  else {
    q = HIGH;
  }
  return q;
}
  
char *cmdDecode(char* command, char* decodedcomm){//writes out in full what alu code is
  
  unsigned short comm = 0b0000;
  for ( int k = 0; k < cmdsize; k++){
    bool newbit = command[k];
    bitWrite(comm,k,newbit);
   }
  switch(comm){
    case 0b0000: decodedcomm = "ROTL"+0;
    case 0b0001: decodedcomm = "SHiL"+0;
    case 0b0010: decodedcomm = "ROTRL"+0;
    case 0b0011: decodedcomm = "SHiR"+0;
    case 0b1100: decodedcomm = "ADDC"+0;
    case 0b1101: decodedcomm = "SUBC"+0;
    case 0b1000: decodedcomm = "AND"+0;
    case 0b0100: decodedcomm = "OR"+0;
    case 0b1010: decodedcomm = "XOR"+0;
    case 0b0110: decodedcomm = "NAND"+0;
    case 0b1001: decodedcomm = "AND"+0;
    case 0b0101: decodedcomm = "OR"+0;
    case 0b1011: decodedcomm = "XOR"+0;
    case 0b0111: decodedcomm = "NAND"+0;
  }
  return decodedcomm;
}

int DigitalConvert(int analogval)
{
  if (analogval > 500) {
    return 1;
  }
  else {
    return 0;
  }
}

char* streamBuffer(char* buff, int wlength) { //reads first wlength bytes in serial buffer
  for ( int i = 0; i <= 64; i++) { //reads entire buffer byte-by-byte
    if (i < wlength) { //stores relevant data and discards the rest
      buff[i] = Serial.read();
      delay(delaytimer); //necessary delay to let all data be stream into serial buffer
    }
    else {
      Serial.read(); //discards irrelevant bytes
    }
  }

  buff[wlength] = 0;//sanity check, cuts string to precise length with EOL/NULL
  return buff;
}


void setup() {
  // put your setup code here, to run once:
  for (int i = Alow; i <= Ahigh; i++) //defines A bus inputs and set bits to 0
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  for (int i = Blow; i <= Bhigh; i++) //defines B bus inputs and set bits to 0
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  for (int i = Outlow; i <= Outhigh; i++) //defines Output bus pins
  {
    pinMode(i, INPUT);
  }
  pinMode (A0, INPUT); //carry
  pinMode (A1, INPUT); //Negative
  pinMode (A2, INPUT); //Zero
  pinMode (A3, INPUT); //Overflow

  Serial.begin(9600); //initiate serial link
  Serial.println("-------------- Intialized -------------");
}

void loop() {
  //allocation and checking of char arrays --> enough space for data and EOL
  Rsptr = (char*) malloc(wordsize+1 * sizeof(char));
  ALUCOMM = (char*) malloc(cmdsize * sizeof(char));
  Roptr = (char*) malloc(wordsize+1 * sizeof(char));
  cmdptr = (char*) malloc(cmdsize+1 * sizeof(char));
  optype = (char*) malloc(cmdsize+1 *sizeof(char));
  
  if (Rsptr == NULL) {
    Serial.println("Memory allocation failure, Rs");
  }
  if (Rdptr == NULL) {
    Serial.print("");
  }
  if (Roptr == NULL) {
    Serial.println("Memory allocation failure, Ro");
  }
  if (cmdptr == NULL) {
    Serial.println("Memory allocation failure, cmd");
  }
  if (optype == NULL) {
    Serial.println("Memory allocation failure, op");
  }


  //data input and bitstring generation
  Serial.println();
  Serial.println("Rs?");
  while (Serial.available() == 0) {} //waits for user input
  Rsptr = streamBuffer(Rsptr, wordsize);
  Serial.println("Ro?");
  while (Serial.available() == 0) {} //waits for user input
  Roptr = streamBuffer(Roptr, wordsize);
  Serial.println("cmd?");
  while (Serial.available() == 0) {} //waits for user input
  cmdptr = streamBuffer(cmdptr, cmdsize);
  

  //lets write to those pins !
  for (int a = 0; a < wordsize; a++) {
    digitalWrite(Alow + a, chartoBool(Rsptr[wordsize - 1 - a])); //REMEMBER: this is not a simple array-index-to-pin translation. The LSB is what we want to associate to pin N0, but
  }
  for (int b = 0; b < wordsize; b++) {
    digitalWrite(Blow + b, chartoBool(Roptr[wordsize - 1 - b])); // it is in fact the last bit of our char array (python: Rs[-1]), so for a 16bit word linked to 16 pins, we want to drecement the bit index (index 15-0)
  }
  for (int c = 0; c < cmdsize; c++) {
    digitalWrite(ALUlow + c, chartoBool(cmdptr[cmdsize - 1 - c])); // while incrementing the pin index.
  }

// reconstruct word for inputs
Rd = 0b100000000000000;
for (int i = 0; i <wordsize; i++){
  bool bittowrite = digitalRead(Outlow+i);
  
  bitWrite(Rd,i,bittowrite);
}

//computing decimal equivalents for A and B and writing results to Serial
optype = cmdDecode(cmdptr, optype);


Serial.println("# # # # # # # # # Results# # # # # # # # # ");
Serial.print("Operand A: ");
Serial.print(Rsptr);
int Adec = bintoDec(Rsptr,16);
Serial.print("          ");
Serial.println(Adec);

Serial.print("Operand B: ");
Serial.print(Roptr);
int Bdec = bintoDec(Roptr,16);
Serial.print("          ");
Serial.println(Bdec);
Serial.print("Operation code: ");
Serial.println(cmdptr);
Serial.println("______________________________");
Serial.print("Result: ");
Serial.print(Rd, BIN);
Serial.print("          ");
Serial.println(Rd);
Serial.println("# # # # # # # # # # # # # # # # # # # # # #" );



//freeing allocated memory
  free(Rsptr);
  free(Roptr);
  free(Rdptr);
  free(cmdptr);
  free(optype);
  
}


