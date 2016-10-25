
int light_pins[9] ={A0, A1, A2, A3, A4, A5, A8, A9, A10};
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
    Serial.println(letter);
  }
}

int calibrate(int colors[2])
{
  int cell_data[9]; 
  int i = 0;
  int white_threshold = 0;
  for(i = 0; i < 9; i++)
  {
    delay(100);
    cell_data[i] = analogRead(light_pins[i]);
    
    white_threshold += cell_data[i];
  }
  delay(30000);
  
  white_threshold = white_threshold / 9;
  
  int j = 0;
  int blue_threshold = 0;
  for(j = 0; j < 9; j++)
  {
    delay(100);
    cell_data[j] = analogRead(light_pins[j]);
    
    blue_threshold += cell_data[j];
  }
  delay(30000);
  blue_threshold = blue_threshold / 9;

  colors[0] = white_threshold;
  colors[1] = blue_threshold;
  return 0;
}

char do_color_detection()
{
  int k;
  int cell_data[9];
  for(k = 0; k < 9; k++)
  {
    delay(100);
    cell_data[k] = analogRead(light_pins[k]);
    Serial.println(cell_data[k]);
  }
  int cells[2];
  calibrate(cells);
  int l;
  double compare_me;
  int white_count = 0;
  for(l = 0; l < 9; l++)
  {
    int white = (cell_data[l] - cells[0]) / cells[0] * 100;
    int blue = (cell_data[l] - cells[1]) /  cells[1] * 100;

    if(white <= blue)
    {
      white_count += 1;
    }

  }

  if(white_count >= 7 )
  {
    return 'M';                         // 1 = M
  }
  if(white_count == 5)
  {
    return 'E';                         // 2 = E
  }
  if(white_count == 4 || white_count == 6)
  {
    return 'A';                         // 3 if A
  }
  return 'T';                           // 4 if T   *no need to check against a value, white count will never be negative, and has to be less than 4, so default it to T.
                                            // We also do not have a case where the block does not have a letter, so doing this is fine.
}


