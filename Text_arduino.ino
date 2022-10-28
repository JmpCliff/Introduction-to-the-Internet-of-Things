char var;
int LED=13;
int RayIn=8;

void setup(){
    pinMode(LED,OUTPUT);
    pinMode(RayIn,INPUT);
    Serial.begin(9600);
    Serial.println("hello");
}

void loop(){
    
    if(digitalRead(RayIn)==LOW){
        digitalWrite(LED,HIGH);
        Serial.println("CATCH!");
        delay(5000);
        digitalWrite(LED,LOW);
    }
    //Serial.println("This is a test!");
    //delay(1000);

}