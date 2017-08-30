# NEATO_LDS_VIRTUAL_BUMPER
Virtual bumper for the lds

A virtual bumper was built to prevent the robot from making as much physical contact with other objects. For this project, a NEATO D5, a gen 2 LDS and an arduino micro was placed inside of the robot. The arduino is reading the LDS scan data directly off of the LDS by connecting the RX on the arduio to test port 6 on the LDS interface board. The arduino is then directly wired to the 4 bumpers on the D5 using a HE721C0510 Non Latching relay to pull the switch either high or low. When the switch is recieving voltage the switch acts as if it was not pressed in. In virtualBumper.ino the left side bumper is set to pin 8 on the arduino, right side is 9, left front is 10, right front is 11.
To parse the data we set the baud rate on the arduino to 115200 and run the getpacket() function. Each packet recieved contains 4 degrees at a time and will be stored into rxbuff. Then, for each packet a for loop is used to access each degree in the packet. To aquire the distance, the dist() function is used.

There is an increasing error in the angle as a point gets closer to the LDS (Azimuth Error due to parallax). To account for this error we take the arcsin of the distance fro mthe center of rotation to the lazer and divide that by the distance. This error changes the angle by about 0.4 to 0.7 degrees. While aquiring data 
X = x*cos(θ) - y*sin(θ)
Y = x*sin(θ) + y*cos(θ)