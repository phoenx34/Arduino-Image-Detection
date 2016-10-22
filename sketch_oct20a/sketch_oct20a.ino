int light_pins[9] ={A0, A1, A2, A3, A4, A5, A6, A7, A8};
int calibrated = 300;
// Calibrate sensors for similarity matching between
// blue and white values. Runs before color detecting/autonomous section
// However, this still can be more accurate because the white color will
// always detect some blue. What percentage do we use?
// At what point between 0 and 255 does blue become white, and vice versa?
// Generally you would avoid this by comparing two resistors per data point,
 // instead of trying to compare each to some "predefined" convergence point
void setup() {

  Serial.begin(9600);
}

void loop() {

  boolean is_block = true; //For now it will detect the block infinitely.
  int block_counter = 0;
  if(is_block == true && block_counter < 5 )
  {
    int letter = do_color_detection();
    block_counter++;
    Serial.println("The block was detected as:");
    //Serial.println(letter);
  }
}

int do_color_detection()
{
  int cell_data[9];                                               // You have to populate an array containing the analog values of the resistance array. Arduino code is stupid.
  int j = 0;
  for(j = 0; j < 9; j++)
  {
    //delay(100);
    cell_data[j] = analogRead(light_pins[j]);
    Serial.println(cell_data[j]);
  }
  int i;
  double compare_me;
  int white_count = 0;
  for(i = 0; i < 9; i++)
  {
    compare_me = (double) similarity(cell_data[i]);

    //Serial.println("Percentage of white:");
    //Serial.println(compare_me);

    if(compare_me > 50 && compare_me < 100)     // No need to check if it is blue, because the int only should grow if the scan is white.
    {
      white_count += 1;
    }

  }

  if(white_count >= 7 )
  {
    return 1;                         // 1 = M
  }
  if(white_count == 5)
  {
    return 2;                         // 2 = E
  }
  if(white_count == 4 || white_count == 6)
  {
    return 3;                         // 3 if A
  }
  return 4;                           // 4 if T   *no need to check against a value, white count will never be negative, and has to be less than 4, so default it to T.
                                            // We also do not have a case where the block does not have a letter, so doing this is fine.
}

double similarity(int cell6)
{
  int toAbs = cell6 - calibrated;
  int diff = abs(toAbs);
  double similar = ((double)diff / (double)calibrated) * 100;
  return similar;
}




