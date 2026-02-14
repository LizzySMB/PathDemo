## Path (final submission)

Please fill this out and submit your work to Gradescope by the deadline.

### Output Comparison

Run the program with the specified `.ini` config file to compare your output against the reference images. The program should automatically save to the correct path for the images to appear in the table below.

If you are not using the Qt framework, you may also produce your outputs otherwise so long as you place them in the correct directories as specified in the table. In this case, please also describe how your code can be run to reproduce your outputs

> Qt Creator users: If your program can't find certain files or you aren't seeing your output images appear, make sure to:<br/>
>
> 1. Set your working directory to the project directory
> 2. Set the command-line argument in Qt Creator to `template_inis/final/<ini_file_name>.ini`

Note that your outputs do **not** need to exactly match the reference outputs. There are several factors that may result in minor differences, such as your choice of tone mapping and randomness.

Please do not attempt to duplicate the given reference images; we have tools to detect this.

|         `.ini` File To Produce Output         |                 Expected Output (No Gamma)                  |                                   Expected Output                                    |                                                                         Your Output                                                                         |
| :-------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------------------------------------------------------------------: |
|         cornell_box_full_lighting.ini         | ![](example-scenes/ground_truth/final/cornell_box_full_lighting_wo_gamma.png)         | ![](example-scenes/ground_truth/final/cornell_box_full_lighting.png)         | ![Place cornell_box_full_lighting.png in student_outputs/final folder](student_outputs/final/cornell_box_full_lighting.png)                 |
|     cornell_box_direct_lighting_only.ini      | ![](example-scenes/ground_truth/final/cornell_box_direct_lighting_only_wo_gamma.png)  | ![](example-scenes/ground_truth/final/cornell_box_direct_lighting_only.png)  | ![Place cornell_box_direct_lighting_only.png in student_outputs/final folder](student_outputs/final/cornell_box_direct_lighting_only.png)          |
| cornell_box_full_lighting_low_probability.ini | ![](example-scenes/ground_truth/final/cornell_box_full_lighting_low_probability_wo_gamma.png) | ![](example-scenes/ground_truth/final/cornell_box_full_lighting_low_probability.png) | ![Place cornell_box_full_lighting_low_probability.png in student_outputs/final folder](student_outputs/final/cornell_box_full_lighting_low_probability.png) |
|                  mirror.ini                   |                  ![](example-scenes/ground_truth/final/mirror_wo_gamma.png)                   |                  ![](example-scenes/ground_truth/final/mirror.png)                   |                                    ![Place mirror.png in student_outputs/final folder](student_outputs/final/mirror.png)                                    |
|                  glossy.ini                   |                  ![](example-scenes/ground_truth/final/glossy_wo_gamma.png)                   |                  ![](example-scenes/ground_truth/final/glossy.png)                   |                                    ![Place glossy.png in student_outputs/final folder](student_outputs/final/glossy.png)                                    |
|                refraction.ini                 |                ![](example-scenes/ground_truth/final/refraction_wo_gamma.png)                 |                ![](example-scenes/ground_truth/final/refraction.png)                 |                                ![Place refraction.png in student_outputs/final folder](student_outputs/final/refraction.png)                                |


> Note: The reference images above were produced using the [Extended Reinhard](https://64.github.io/tonemapping/#extended-reinhard) tone mapping function with minor gamma correction. You may choose to use another mapping function or omit gamma correction.

### Implementation Locations

Please link to the lines (in GitHub) where the implementation of these features start:

- [Diffuse Reflection](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L287)
- [Glossy Reflection](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L248)
- [Mirror Reflection](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L242)
- [Refraction (with Fresnel refletion)](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L159)
- [Soft Shadows](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L395)
- [Illumination](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L109)
- [Russian Roulette path termination](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L145)
- [Event Splitting](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L140)
- [Tone Mapping](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L84)
- [Importance Sampling](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L319) [pdf here](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L268)
- [Depth of Field](https://github.com/brown-cs-224/path-LizzySMB/blob/0a2a78706731c68a299aa17c413c14f1df9bbdb4/pathtracer.cpp#L58)
- [Attenuate Refracted Paths](https://github.com/brown-cs-224/path-LizzySMB/blob/9abfe56aa43d64077cabee75cab451ee3ba1f4a0/pathtracer.cpp#L210)

### Design Choices

Please list all the features your path tracer implements.
Diffuse, Glossy, and Mirror Reflections, Refraction with Fresnel reflection, Soft Shadows, Indirect Illumination, Direct Lighting, Russian Roulette Path Termination, Event Splitting, Tone Mapping, Importance Sampling, Depth of Field, and Refracted Path Attenuation.

### Extra Features

Briefly explain your implementation of any extra features, provide output images, and describe what each image demonstrates.

I implemented importance sampling for clearer images. Here is an example of regular vs. importance sampling on the glossy image.

<img width="512" height="512" alt="glossy_no_is" src="https://github.com/user-attachments/assets/94601c5d-ef2e-4a37-beb6-dd39ee7f2da4" />
<img width="512" height="512" alt="glossy" src="https://github.com/user-attachments/assets/d64003ab-d3bf-4c1a-a6bc-64ac86cc24ee" />


I tried to implement diffuse brdf importance sampling as well, but I'm honestly not sure if what I have there is just regular sampling.

I also used Beer-Lambert absorption to implement attenuated refracted paths. Here's a before and after: 
<img width="512" height="512" alt="refraction_no_attenuation" src="https://github.com/user-attachments/assets/52ce926e-f9e6-4b35-956a-5af28fd8c798" />
<img width="512" height="512" alt="refraction" src="https://github.com/user-attachments/assets/6bd38236-7ea0-4098-8a6c-5ed610c0f6ae" />

For depth of field, I added a lens and focal distance that the user can adjust to create different blurs and focuses as the virtual film plane and lens radius are adjusted. Some examples:

Focal distance: 10, Lens radius: 0.01
<img width="512" height="512" alt="lens  01, foc 10" src="https://github.com/user-attachments/assets/2dd0df31-6da9-42dc-ab88-56852544c2c4" />

Focal distance: 10, Lens radius: 0.1
<img width="512" height="512" alt="lens  1, foc 10" src="https://github.com/user-attachments/assets/4af77323-592f-47c5-8c42-c01002e0969d" />

Focal distance: 10, Lens radius: 1
<img width="512" height="512" alt="lens 1, foc 10" src="https://github.com/user-attachments/assets/41e624e5-d302-4704-8911-efc1ddf1924a" />

Focal distance: 1, Lens radius: 0.1
<img width="512" height="512" alt="lens  1, foc 1" src="https://github.com/user-attachments/assets/b5aab306-d3bc-4d1c-8514-c0c44ec44438" />

Focal distance: 0.5, Lens radius: 0.1
<img width="512" height="512" alt="lens  1, foc  5" src="https://github.com/user-attachments/assets/89983940-fd40-4a02-8943-7384f75d867e" />


### Collaboration/References
Beer-Lambert: https://www.geeksforgeeks.org/physics/beer-lambert-law/

### Known Bugs
N/A
