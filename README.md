# Oscar Perez Assignment 3 Advanced Graphics Programming
This project has been done by Oscar Pérez Martín.

You can view the source code at: https://github.com/oscarpm5/AdvancedGraphicsProgrammingBaseEngine
A release with the compiled demo with only the executable and the models for an easy plug-and-play demonstration can be found in the release section of the github repository or through this link: TODO


## Controls

A simplified controls information window is also avaliable when executing the release.

* **F** (Press): Resets the camera to its initial position, rotation and zoom.

### Movement
Camera movement is performed in the directions of the camera local coordinates.
* **W** (Press/Hold): Move forward.
* **S** (Press/Hold): Move backward.
* **A** (Press/Hold): Move to the left.
* **D** (Press/Hold): Move to the right.
* **E** (Press/Hold): Move up.
* **Q** (Press/Hold): Move down.
* **SPACE** (Hold): Multiply movement speed.

### Orbit
* **MOUSE RIGHT CLICK** (Hold): Orbit mode.
* **MOUSE WHEEL** (Scroll): Changes the distance of the camera from the target point (Zoom-In/Out).
* **MOUSE MOVEMENT**: When in orbit mode, orbits the camera around a target point.


## Techniques
The project supports two different pipelines: **forward** and **deferred**. The two effects implemented are avaliable for the deferred pipeline.
The pipeline being used can be changed through the debug ImGui window when executing the release. 

This is the default look of the image without effects on the deferred pipeline:
![alt text][Base]

When on the deferred pipeline both techniques implemented can be activated through a checkbox in the inspector to show their properties. The techniques implemented are the following:

### SSAO

This effect can be activated by simply checking the SSAO effect checkbox in the ImGui inspector, which appears if the rendering pipeline is the deferred pipeline.

![alt text][SSAO_Widget]

When active some options to configure the effect appear as seen in the image above and can be changed in real time. The kernel radius specifies how far away neighbouring samples are taken. The bias offsets the kernel samples hemisphere along the surface normal and incrementing this value can help get rid of some artifacts with very flat surfaces.

When the blur checkbox is active the noisy SSAO image created by a limited ammount of samples are smoothed with a simple square blur. The size of the blur kernel can be adjusted with the blur ammount parameter that appears once the blur checkbox is checked.

In the table below you can compare the results with and without blur in the SSAO effect:

| SSAO with noise | SSAO blured |
| ------------- | ------------- |
| ![alt text][SSAO_Noisy]  | ![alt text][SSAO_Blured]  |

The shader code for both the noisy SSAO and the square blur to smooth it out can be found in the [ssao.glsl](Engine/WorkingDir/ssao.glsl) file.


### Bloom
![alt text][Bloom] ![alt text][Bloom_Widget]

![alt text][Final]


Changing the display of the different render targets can be done through the ImGui UI.




[Base]: Images/Base_No_Effects.PNG "Base image with no effects"
[SSAO_Noisy]: Images/SSAO_No_Blur.PNG "Final image with SSAO applied. The SSAO effect is noisy"
[SSAO_Blured]: Images/SSAO_Blurred.PNG "Final image with SSAO with blur applied"
[SSAO_Widget]: Images/SSAO_Widget.PNG "SSAO parameters customizable through the editor"
[Bloom]: Images/Bloom.PNG "Bloom effect applied in the final image"
[Bloom_Widget]: Images/Bloom_Widget.PNG "Bloom parameters customizable through the editor"
[Final]: Images/All_Effects.PNG "Final image with both SSAO blured and Bloom applied"


