# CS330 - Computational Graphics and Visualizations
**CS330 Final Project**
<br/>
![image](https://user-images.githubusercontent.com/116769623/204107661-8563b7fa-3e2c-4a3b-afcb-e3b6e85d7304.png)
<br/>
This project's goal was to recreate the scene above using the OpenGL API. For this project, the goal was to recreate as much of the scene as possible, while staying within the bounds of the scope for this project. The objects are modeled using a combination of basic shapes. The hedges are recreated with cubes and pyramids. The bushes are modeled using spheres. The centerpiece is created using a cube, and 2 pyramids with some textures. And finally, the XZ plane is the ground.
<br/>

-----

**Justification for development choices:**
<br/>
To model a scene using the entry-level tools that we studied this term; some concessions and creativity was required.  The first (and arguably most noticeable) is the centerpiece flower arrangement. Using basic shapes, it is very hard to accurately model a bouquet of flowers. For this I decided that a sphere was the closest shape. With the correct texture applied, the result turned out well enough to tell what it is. Given more time and resources, I would have preferred to model each flower individually. A second design decision was to replace the quarter circle hedges with squared ones. When using a torus, or curved shape, the textures for the hedges became mis-aligned. It looks much better and adds some satisfying symmetry by using rotated hedge objects.

-----

**Navigation:**
<br/>
The program uses input from both a mouse and keyboard. These inputs are taken in real-time and allow for pan, yaw, altitude, and scroll speed adjustments. This allows for a natural and smooth navigation experience. Most of this functionality was provided by the camera header class. By calling the process input function, the camera class was able to use commands correctly. 
<br/>
**Basic controls are as follows:**
<br/>
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

**Custom Functions:**
<br/>
Using the guidance of the tutorials, my entire code is modularized. This means that for future projects I can easily use the same code backbone. Everything from shaders, textures, the render loop, and more are all broken down into their own respective functions. This also helped with adding new objects into the program. Since everything is modularized, it was simple to take a step-by-step approach when adding new objects or features. Also, with the code setup the way it is, and once I learn more advanced features, I can start to experiment with more mesh layers and advanced lighting techniques. New things to learn include raytracing and importing scanned models.
