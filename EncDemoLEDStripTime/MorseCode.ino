String morseCon() {
  Serial.println("morseCon() called"); 
  
  String morsey = "";  
  String sentence = morse(); 
  Serial.print("sentence is: " + sentence); 

  for (int i = 0; i < sentence.length(); i++) {
    morsey += letter(sentence.charAt(i)) + " ";  
//    Serial.print(morse[i]); 
//    Serial.print(" "); 
  }
  return morsey; 
}

String letter(char let) {
  Serial.println("letter() called"); 
  switch (let) {
    case 'a':
      return ".-";
    case 'b': 
      return "-..."; 
    case 'c': 
      return "-.-."; 
    case 'd': 
      return "-.."; 
    case 'e': 
      return "."; 
    case 'f': 
      return "..-."; 
    case 'g': 
      return "--."; 
    case 'h': 
      return "...."; 
    case 'i': 
      return ".."; 
    case 'j': 
      return ".---"; 
    case 'k': 
      return "-.-"; 
    case 'l': 
      return ".-.."; 
    case 'm': 
      return "--"; 
    case 'n':
      return "-."; 
    case 'o': 
      return "---"; 
    case 'p': 
      return ".--."; 
    case 'q': 
      return "--.-"; 
    case 'r': 
      return "-.-"; 
    case 's': 
      return "..."; 
    case 't': 
      return "-";  
    case 'u': 
      return "..-"; 
    case 'v': 
      return "...-"; 
    case 'w': 
      return ".--"; 
    case 'x': 
      return "-..-"; 
    case 'y': 
      return "-.--"; 
    case 'z': 
      return "--.."; 
    default: 
      return " "; 
  }
}

String morse() {
  Serial.println("morse() called"); 
  String phrases[] = { "hello how are you doing",
    "this is a trasmition",
    "leds are cool",
    "rockets are really cool",
    "qwerty" };

  //return phrases[random(0,sizeof(phrases))]; 
  return "hello"; 
}
