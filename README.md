# CS330 - Computational Graphics and Visualizations
![image](https://user-images.githubusercontent.com/116769623/204107661-8563b7fa-3e2c-4a3b-afcb-e3b6e85d7304.png)
<br/>
<br/>
## **CS330 Final Project**
<br/>
This project's goal was to recreate the scene above using the OpenGL API. For this project, the goal was to recreate as much of the scene as possible, while staying within the bounds of the scope for this project. The objects are modeled using a combination of basic shapes. The hedges are recreated with cubes and pyramids. The bushes are modeled using spheres. The centerpiece is created using a cube, and 2 pyramids with some textures. And finally, the XZ plane is the ground. A video of the final product is below:
<br/>
<br/>

https://user-images.githubusercontent.com/116769623/204109492-df7a5a89-622d-4929-bfa3-c236414e1e0d.mp4

-----

### Justification for development choices:
<br/>
To model a scene using the entry-level tools that we studied this term; some concessions and creativity was required.  The first (and arguably most noticeable) is the centerpiece flower arrangement. Using basic shapes, it is very hard to accurately model a bouquet of flowers. For this I decided that a sphere was the closest shape. With the correct texture applied, the result turned out well enough to tell what it is. Given more time and resources, I would have preferred to model each flower individually. A second design decision was to replace the quarter circle hedges with squared ones. When using a torus, or curved shape, the textures for the hedges became mis-aligned. It looks much better and adds some satisfying symmetry by using rotated hedge objects.

-----

### Navigation:
<br/>
The program uses input from both a mouse and keyboard. These inputs are taken in real-time and allow for pan, yaw, altitude, and scroll speed adjustments. This allows for a natural and smooth navigation experience. Most of this functionality was provided by the camera header class. By calling the process input function, the camera class was able to use commands correctly. 
<br/>
<br/>
**Basic controls are as follows:**
<br/>
W – Forward
<br/>
S – Backwards
<br/>
A – Pan Left
<br/>
D – Pan Right
<br/>
Q – Altitude down
<br/>
E – Altitude UP
<br/>
L – Start Sun rotation
<br/>
K – Stop Sun rotation
<br/>
Mouse – Pitch and yaw controls
<br/>
Scrolling – speed control for all inputs

-----

### Custom Functions:
<br/>
Using the guidance of the tutorials at https://learnopengl.com/, my entire code is modularized. This means that for future projects I can easily use the same code backbone. Everything from shaders, textures, the render loop, and more are all broken down into their own respective functions. This also helped with adding new objects into the program. Since everything is modularized, it was simple to take a step-by-step approach when adding new objects or features. Also, with the code setup the way it is, and once I learn more advanced features, I can start to experiment with more mesh layers and advanced lighting techniques. New things to learn include raytracing and importing scanned models.

-----

### Enhancements

This project was recently enhanced according to the enhancements found in this [code review](https://www.youtube.com/watch?v=Pwx_MJLvxNQ).

For my first enhancement I worked on the CS 330 Computation Graphics and Visualization final project. This project was started in October of 2021 and was one of the first complex programs that I had to work on. This program utilizes several other sources of code created by others and successfully puts them together to achieve the intended result. By doing this with consistency, an easy-to-follow structure, and comments that will allow any coder to understand the program’s logic, I have demonstrated abilities in Software Engineering and Design. Specifically, the structure of the program allows for future objects to be easily added with the intention of adding detail to the scene. I demonstrated this through my original code and it’s enhancements, which include: two new hedges, the Sun revolving around the origin, a re-worked center road object, a skybox, refined comments, redundant code removal, style consistency refined, and security modifications.

Adding these objects required a few different strategies. For the hedges I used the existing outer hedge object. This object was scaled and then translated to the position it is in now, and this enhancement was done with only 5 lines of code (efficiency). To create the motion of the sun, I added an angular velocity to the object’s position allowing it to rotate around the origin. Then I added keyboard functionality to pause the motion. The “L” key will start the motion and the “K” key will stop the motion. While adding these enhancements to the project, the mismatched textures of the center brick road were too obvious. So, I created a new object with its own texture coordinates to avoid reusing the same road object. Lastly, the blank backdrop was apparent, so I added a simple skybox. 

As this was my first complex program, naturally there was over-top commenting and redundant code. For example, when adding additional objects before, there was a lot of resetting variables that were already set, or repeated variables due to re-used objects. I cleaned this up and added comments to make it easier to follow. There was also some style (spaces, indents, newlines, etc.) that needed to be straightened out for consistency. And finally, I added in default else statements and caught a couple of memory leaks for added security and efficiency.

#### Video of project prior to enhancements

https://user-images.githubusercontent.com/116769623/206036720-03bf4aee-b982-4538-bdb1-d05ae9a45254.mp4
