Arduino-Image-Detection
Use: 
Download any/all the .ini files. Those are arduino files. Extract them to your desktop and then find them with arduino. The isuckatcode.ino is the file with all of the code in it. It compiles, but I have not done any structuring, and it will definately not work as intended right now. 

Image detection: 
Download pathfind.ino
In the loop, you can uncomment prints to output the letter it detected. The threshold value right now is just an int called "calibrated" at the very top. There are useful prints throughout the code. To get the correct threshold, you have to uncomment the print statement inside the loop going through the resistors. Leave it running for at least a minute with a blue-only block on the sensor, inside the arm, pointing down on a black surface. You should see a general, rough trend where there are two groups of similar values. Matt helped me debug yesterday, he understands the data. 

Autonomous Drive: 
Currently the autonomous drive is located inside isuckatcode. I will work on extracting it at some point tonight. 

General Notes and Guidelines as to what to test:
Shashank never sent me the updated driving/controller code. We know it drives properly, and we know that code indeed works (granted the controller connects to the bluetooth). Do not worry about that. I need you to test anything we didnt get a chance to, so we have data for when I will code after the robot ships. I need a bunch of image detection data; at least the block detected at each of the different letters, and as many groups of the 9 raw resistor values you can get me. I need final pin numbers for the entire setup. Just something like PWM1 = rotating arm servo, etc. For the automation, I can walk you through what is going on but for the most part all we would need to change is it's threshold value, just like the pathfind. 

Save autonomous driving to test last. The servo for rotation of the circular spinny block sort thing is not written. I need to also upload a copy of the basic servo code to control the one arm. We can base the rest of the servo code off of this one; they need to follow the same delays and setup. 
