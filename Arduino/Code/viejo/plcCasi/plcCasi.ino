//---------------------------- BLOQUE DE LIBRERÍAS --------------------------//

#include <EEPROM.h>
#include <ArduinoJson.h>


//------------------------ BLOQUE DE CONTROL DE TIEMPO ----------------------//

unsigned long timeMaxC; // tiempo m�ximo para calibraci�n del home del carrusel
unsigned long tiempoC; //tiempo de calibracion del sensor del carrusel
unsigned long timeMaxH; // tiempo m�ximo para calibraci�n del home de la herramienta
unsigned long timeMaxB; // tiempo m�ximo para calibraci�n del home del brazo

// unsigned long timePLC; // tiempo de respuesta m�xima para que el plc ejecute la acci�n


int timeTool = 1960; //1956 tiempo entre cada herramienta
int tiempoEspera = 500; // tiempo de espera entre cada cambio de herramienta 
unsigned long tiempoSerial; // tiempo de espera serial para recibir la respuesta del otro PLC

//----------------- BLOQUE DE CONTROL DE INDICADORES BANDERA ----------------//

int flagC = 0; 
int flagH = 0;
int flagB = 0;


//----------------------- BLOQUE DE ASIGNACI�N DE PINES ---------------------//

// Sensores
int sensorC = 7; // Sensor de efecto hall del carrusel
int sensorH = 6; // Sensor de final de carrera del mandril de herramienta
int sensorB = 5; // Sensor de efecto hall del brazo

// Paso de los servomotores (NO MODIFICAR NINGUNO DE ESTOS PINES)
int pasoC = 4 ; // Salida al servo motor del carrusel
int pasoH = 13; // Salida al servo motor del mandril de la herramienta
int pasoB = 12; // Salida al servo motor del brazo

// Direcci�n de los servomotores
int direccionC = 8 ; // direcci�n del motor del carrusel
int direccionH = 9 ; // direcci�n del motor del mandril de la herramienta
int direccionB = 11; // direcci�n del motor del brazo

// Comunicacion con PLC
int tx2 = 17;  // Trasmisi�n de datos
int rx2 = 16;  // recepci�n de ejecuci�n

/*
UTP1
cafe -> Ground
azul -> 13 paso del mandril de la herramienta
verde -> 12 Paso servo brazo
naranja -> 11 direccion motor brazo
azul/b -> 9 direccion motor mandril herramienta
naranja/b -> 8 direccion motor carrusel

UTP2
Cafe -> 7 sensor de efecto hall del carrusel
Cafe/b -> 6 sensor de final de carreradel madril
Verde -> 5 sensor del brazo
Verde/b -> 4 paso del carrusel 
Azul -> tx3
Azul/b -> rx3
naranja -> tx2
naranja/b -> rx2
*/


//---------------------- BLOQUE DE DETECCI�N DE SENSORES --------------------//

int accionC = 1; 
int accionH = 1;
int accionB = 1;
int accionS = 1;

// int accionPLC = 1;
int detectTinyG = 0;


//--------------------- VARIABLES DE CONTROL DEL CARRUSEL -------------------//

int directionTool = 0;
int tool = EEPROM.read(directionTool); // herramienta en posicion de home
int validationHome = 0;
int changeNumber = 0;


//---------------------- FUNCIÓN DE CONFIGURACIÓN INICIAL -------------------//

void setup(){
  Serial.begin(9600); 
  //Serial3.begin(9600);
  //Serial2.begin(9600);

  pinMode(sensorC, INPUT);
  pinMode(pasoC, OUTPUT);
  pinMode(direccionC, OUTPUT);
  pinMode(sensorH, INPUT);
  pinMode(pasoH, OUTPUT);
  pinMode(direccionH, OUTPUT);
  pinMode(sensorB, INPUT);
  pinMode(pasoB, OUTPUT);
  pinMode(direccionB, OUTPUT);
  pinMode(tx2,OUTPUT);
  pinMode(rx2,INPUT);

  digitalWrite(pasoC, LOW);
  digitalWrite(pasoH, LOW);
  digitalWrite(pasoB, LOW);

  digitalWrite(direccionC,LOW);
  digitalWrite(direccionH,LOW);
  digitalWrite(direccionB,LOW);

  digitalWrite(tx2,LOW);  
}


//-------------------------- FUNCIÓN DE LOOP PRINCIPAL ----------------------//

void loop() {
  if(Serial.available()){
    
    if(detectTinyG == 0){
      while(Serial.available()){
        char x = Serial.read();
        //x = 0;
        delay(100);
        //Serial.flush();
      }
      detectTinyG = 1;
      Serial.flush();
    }
    
    char c = Serial.read();
    switch(c){
      case 'a': // home de todas las herramientas
        homeTool();
        break;
      case 'T':
        changeTool();
        break;
      //case 'b': //enganchar
      //  changeB(1,0);
      //  break;
      //case 'c': //desenganchar
      //  changeB(1,1);
      //  break;
      /*case 'd': //girar media vuelta
        changeB(2,0);
        break;
      case 'e': //mover la herramienta a 150cm de la posición de trabajo
        changeH(150.0);
        break;
      case 'f': //home del carrusel
        homeC();
        break;
      case 'g': //home del braxo
        homeB();
        break;*/
      case 'i': //mover carrusel a la herramienta seleccionado, agregar el entero de 1-18 seguido del h
        changeToolWeb();
        break;
      case 'h':
        changeNextTool();
        break;

    }
  }
}

//------------------- FUNCIÓN CAMBIO DE HERRAMIENTA CARRUSEL ----------------//

void changeNextTool(){
  char j = Serial.parseInt();
  if(j > 0 && j <= 18){
    delay(8000);
    changeC(j);
  }
}

void changeToolWeb(){
  char j = Serial.parseInt();
  if(j > 0 && j <= 18){
    changeC(j);
  }
}



//------------------------ FUNCIÓN DE CODIFICACIÓN JSON ---------------------//

void sendJson(String msg, int status){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["msg"] = msg;
  root["status"] = status;
  root.printTo(Serial);
  Serial.println();
}


//--------------------- FUNCIÓN HOME DE TODOS LOS MOTORES -------------------//

void homeTool(){
  int secuencia = 0;
  validationHome = 0;
  secuencia = homeH();
  if(secuencia == 1){
    secuencia = 0;
    secuencia = homeC();
    if(secuencia == 1){
      secuencia = 0;
      secuencia = homeB();
      if(secuencia == 1){
        if(EEPROM.read(directionTool) != 1){
          EEPROM.write(directionTool,1);
        }
        sendJson("a",1);
        validationHome = 1;
        changeNumber = 0;
      }
      else sendJson("c",0);
    }
    else sendJson("b",0);
  }
  else sendJson("d",0);
}


//------------------------- FUNCIÓN HOME DEL CARRUSEL -----------------------//

int homeC(){
  accionC = 1;
  TCCR0B = TCCR0B & 0b1111000 | 0x03;
  analogWrite(pasoC,5);
  int estadoH = 0;
  int distanciaHome = abs(1 - tool);
  flagC = 0;
  if(tool == 18){
    estadoH = 0;
  }
  else if(distanciaHome <= 9){
    estadoH = 0;
  }
  else {
    estadoH = 1;
  }
  digitalWrite(direccionC, 1 - estadoH);
  timeMaxC = millis();
  while(accionC){
    if(digitalRead(sensorC) == HIGH && flagC == 0){
      digitalWrite(pasoC, LOW);
      TCCR0B = TCCR0B & 0b1111000 | 0x04;
      delay(100);
      tiempoC = millis();
      analogWrite(pasoC, 5);
      flagC = 1;
    }
    else if(digitalRead(sensorC) == LOW && flagC == 1){
      digitalWrite(pasoC, LOW);
      tiempoC = millis() - tiempoC;
      digitalWrite(direccionC, estadoH);
      delay(100);
      analogWrite(pasoC, 5);
      delay(tiempoC*3/5);
      digitalWrite(pasoC, LOW);
      accionC = 0;
      flagC = 0;
      delay(200);
      digitalWrite(pasoC, LOW);
      TCCR0B = TCCR0B & 0b1111000 | 0x03;
      digitalWrite(direccionC, LOW);
      analogWrite(pasoC, 5);
      delay(2990); //2980tiempo para llegar del sensor al home con la primera herramienta
      digitalWrite(pasoC, LOW);
      tool = 1;
      //sendJson("Herramienta 1 en Home",1);
      return 1;
      break;
    }
    if(millis() - timeMaxC >= 40000){
      accionC = 0;
      digitalWrite(pasoC, LOW);
      //sendJson("No se pudo detectar el sensor del carrusel",0);
      return 0;
    } 
  }
}


//----------------- FUNCIÓN HOME DEL MANDRIL DE LA HERRAMIENTA --------------//

int homeH(){
  accionH = 1;
  flagH = 0;
  TCCR0B = TCCR0B & 0b1111000 | 0x02;
  digitalWrite(direccionH,HIGH);
  analogWrite(pasoH,5);
  timeMaxH = millis();
  while(accionH){
    if(digitalRead(sensorH) == HIGH && flagH == 0){
      digitalWrite(pasoH, LOW);
      TCCR0B = TCCR0B & 0b1111000 | 0x03;
      flagH = 1;
    }
    else if(digitalRead(sensorH) == LOW && flagH == 1){
      delay(1000);
      digitalWrite(direccionH,LOW);
      analogWrite(pasoH,5);
      delay(400); //tiempo de calibraci�n, 740 originalmente
      digitalWrite(pasoH, LOW);
      accionH = 0;
      flagH = 0;
      return 1;
    }
    if((millis() - timeMaxH) > 28000){
      accionH = 0;
      digitalWrite(pasoH, LOW);
      return 0;
    }
  }
}


//------------------------- FUNCIÓN HOME DEL BRAZO -----------------------//

int homeB(){
  accionB = 1;
  flagB = 0;
  TCCR1B = TCCR1B & 0b1111000 | 0x02;
  digitalWrite(direccionB,LOW);
  analogWrite(pasoB,5);
  timeMaxB = millis();
  while(accionB){
    if(digitalRead(sensorB) == HIGH && flagB == 0){
      delay(120);//encontrar el sensor en el centro
      digitalWrite(pasoB, LOW);
      flagB == 1;
      accionB = 0;
    }
    if(millis() - timeMaxB >= 7000){
      accionB = 0;
      digitalWrite(pasoB, LOW);
      digitalWrite(direccionB, LOW);
      return 0;
    }
  }
  TCCR1B = TCCR1B & 0b1111000 | 0x01;
  delay(1000);
  digitalWrite(direccionB,HIGH);
  analogWrite(pasoB,5);
  delay(680); //650
  digitalWrite(pasoB, LOW);
  digitalWrite(direccionB, LOW);
  return 1;
}


//------------ FUNCIÓN CAMBIO DE HERRAMIENTA DE TODOS LOS MOTORES -----------//


void changeTool(){
  delay(5);
  int a = Serial.parseInt();
  delay(3);
  char b = Serial.read();
  delay(5);
  float c = Serial.parseFloat();
  if(a >= 1 && a <= 18){ // validar el numero de la herramienta
    if(b == ';'){ // validar el car�cter limitador
      if(c >=0 && c <=300){ // validar la longitud maxima del offset
        int secuencia = 0;
        if(validationHome == 1){ // validar que anteriormente se realiz� la calibraci�n
          if(changeNumber >= 20){ // numero maximo de cambios de herramienta que puede realizar
            homeTool();
            changeNumber = 0;
          }
          secuencia = homeH(); // hacer home en la herramienta del motor
          if(secuencia == 1){ 
            secuencia = 0;
            delay(tiempoEspera);
            secuencia = changeC(a); 
            if(secuencia == 1){
              secuencia = 0;
              delay(tiempoEspera);
              secuencia = changeB(1,0); // enganchar las herramientas con el brazo
              if(secuencia == 1){ //Continuar con la accion al plc
                secuencia = 0;
                delay(tiempoEspera);
                secuencia = comunicationPLC(1); // acci�n para bajar los pistones
                if(secuencia == 1){
                  secuencia = 0;
                  delay(tiempoEspera+1500);
                  secuencia = changeB(2,0); // cambio de herramienta
                  if(secuencia == 1){ 
                    secuencia = 0;
                    delay(tiempoEspera);
                    // Serial.print("a0");
                    // Serial.setTimeout(tiempoSerial*2);
                    // secuencia = Serial.parseInt();
                    secuencia = comunicationPLC(0); // acci�n para subir los pistones
                    if(secuencia == 1){
                      secuencia = 0;
                      delay(tiempoEspera+1000);
                      secuencia = changeB(1,1); // regresar el brazo a la posicion inicial
                      if(secuencia == 1){
                        secuencia = 0;
                        delay(tiempoEspera);
                        secuencia = changeH(c); // llevar herramienta a la posici�n de trabajo
                        if(secuencia == 1){
                          secuencia = 0;
                          // int x = 0;
                          sendJson("e",1);
                          changeNumber ++;
                        }
                        else sendJson("f",0);
                      }
                      else sendJson("g",0);
                      //}
                      //else sendJson("h",0);
                    }
                    else sendJson("i",0);
                  }
                  else sendJson("j",0);
                }
                else sendJson("k",0);
                //}
                //else sendJson("l",0);
              }
              else sendJson("m",0);
            }
            else sendJson("n",0);
          }
          else sendJson("o",0); 
        }
        else sendJson("p",0);
      }
      else sendJson("q",0);
    }
    else sendJson("r",0);
  }
  else sendJson("s",0);
}



//------------------------ FUNCIÓN CAMBIO DEL CARRUSEL ----------------------//

int changeC(int newTool){
  TCCR0B = TCCR0B & 0b1111000 | 0x03;
  int distancia = abs(newTool - tool);
  
  if(distancia <= 9){
    if((newTool - tool)>0){
      digitalWrite(direccionC, LOW);
    }
    else digitalWrite(direccionC, HIGH);
  }
  else {
    distancia = 18 - distancia;
    if((newTool - tool)>0){
      digitalWrite(direccionC, HIGH);
    }
    else digitalWrite(direccionC, LOW);
  }
  analogWrite(pasoC,5);
  delay(distancia*timeTool); // tiempo entre cada herramienta
  digitalWrite(pasoC, LOW);
  EEPROM.write(directionTool,newTool);    
  tool = newTool;
  //sendJson("Herramienta "+String(tool)+" en Home",1);
  return 1;
}


//------------------------ FUNCIÓN CAMBIO DEL BRAZO ----------------------//

int changeB(int giro, int sentido){
  TCCR1B = TCCR1B & 0b1111000 | 0x01;
  if(giro == 2){
    giro *= 816;
  }else giro = 810;
  digitalWrite(direccionB, sentido); //0 en sentido antihorario, 1 sentido horario 
  analogWrite(pasoB,5);
  delay(giro); // tiempo de espera
  //delayMicroseconds(1000);
  digitalWrite(pasoB, LOW);
  return 1;
}


//--------------- FUNCIÓN CAMBIO DEL MANDRIL DE LA HERRAMIENTA --------------//

int changeH(double offset){
  delay(5);
  double value = -83.5143*offset + 26024.1356;
  TCCR0B = TCCR0B & 0b1111000 | 0x02;
  digitalWrite(direccionH, LOW);
  analogWrite(pasoH,5);
  delay(value); // tiempo de espera
  digitalWrite(pasoH, LOW);
  return 1;
}


//---------------------------- COMUNICACI�N PLC -----------------------------//

int comunicationPLC(int function){
  accionS = 1;
  if(function == 1){  // accionar pistones
    Serial.println("a");
  }
  else Serial.println("b");
  tiempoSerial = millis();
  while(accionS){
    if(Serial.available()){
      char response = Serial.read();
      //Serial.println(response);
      if(response == '1'){
        accionS = 0;
        return 1;
      }
    }
    else if((millis() - tiempoSerial) > 18000){
      accionS = 0;
      return 0;
    }
  }
}


//----------------------------------- END -----------------------------------//

