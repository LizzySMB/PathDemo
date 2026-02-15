### README

Path tracing is a simple, elegant Monte Carlo sampling approach to solving the rendering equation. Like ray tracing, it produces images by firing rays from the eye/camera into the scene. Unlike basic ray tracing, it generates recursive bounce rays in a physically accurate manner, making it an unbiased estimator for the rendering equation. Path tracers support a wide variety of interesting visual effects (soft shadows, color bleeding, caustics, etc.), though they may take a long time to converge to a noise-free image.



This path tracer uses Monte Carlo sampling with Russian Roulette path termination to render 3D scenes, providing indirect illumination and direct lighting with physically accurate recursive bounce rays that provide soft shadows, color bleeding, caustics, etc. This implementation includes support for diffuse and specular BRDFs, reflection, refraction with Fresnel reflection, refracted path attentuation, and more (longer feature list below).


### Output Comparison


|         `.ini` File To Produce Output         |                 Base Output (No Gamma)                  |                                   Base Output (Gamma Corrected)                                    |                                                                         Actual Output with add-ons                                                                         |
| :-------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------------------------------: | :---------------------------------------------------------------------------------------------------------------------------------------------------------: |
|         cornell_box_full_lighting.ini         | ![](example-scenes/ground_truth/final/cornell_box_full_lighting_wo_gamma.png)         | ![](example-scenes/ground_truth/final/cornell_box_full_lighting.png)         | ![Place cornell_box_full_lighting.png in student_outputs/final folder](student_outputs/final/cornell_box_full_lighting.png)                 |
|     cornell_box_direct_lighting_only.ini      | ![](example-scenes/ground_truth/final/cornell_box_direct_lighting_only_wo_gamma.png)  | ![](example-scenes/ground_truth/final/cornell_box_direct_lighting_only.png)  | ![Place cornell_box_direct_lighting_only.png in student_outputs/final folder](student_outputs/final/cornell_box_direct_lighting_only.png)          |
| cornell_box_full_lighting_low_probability.ini | ![](example-scenes/ground_truth/final/cornell_box_full_lighting_low_probability_wo_gamma.png) | ![](example-scenes/ground_truth/final/cornell_box_full_lighting_low_probability.png) | ![Place cornell_box_full_lighting_low_probability.png in student_outputs/final folder](student_outputs/final/cornell_box_full_lighting_low_probability.png) |
|                  mirror.ini                   |                  ![](example-scenes/ground_truth/final/mirror_wo_gamma.png)                   |                  ![](example-scenes/ground_truth/final/mirror.png)                   |                                    ![Place mirror.png in student_outputs/final folder](student_outputs/final/mirror.png)                                    |
|                  glossy.ini                   |                  ![](example-scenes/ground_truth/final/glossy_wo_gamma.png)                   |                  ![](example-scenes/ground_truth/final/glossy.png)                   |                                    ![Place glossy.png in student_outputs/final folder](student_outputs/final/glossy.png)                                    |
|                refraction.ini                 |                ![](example-scenes/ground_truth/final/refraction_wo_gamma.png)                 |                ![](example-scenes/ground_truth/final/refraction.png)                 |                                ![Place refraction.png in student_outputs/final folder](student_outputs/final/refraction.png)                                |


### Implementation Locations

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
- [Stratified Sampling](https://github.com/brown-cs-224/path-LizzySMB/blob/b52056d97aa83fe9e3135a6fb000625ab7d5c342/pathtracer.cpp#L33)

### Additional Features

I implemented importance sampling for clearer images. Here is an example of regular vs. importance sampling on the glossy image.

<img width="512" height="512" alt="glossy_no_is" src="https://github.com/user-attachments/assets/94601c5d-ef2e-4a37-beb6-dd39ee7f2da4" />
<img width="512" height="512" alt="glossy" src="https://github.com/user-attachments/assets/d64003ab-d3bf-4c1a-a6bc-64ac86cc24ee" />


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


Stratified sampling; not sure about the effectiveness, but there are slight differences. Before and after:

<img width="512" height="512" alt="glossy no strat" src="https://github.com/user-attachments/assets/de8b427c-1251-4b7c-b01a-14e764dfa0fb" />
<img width="512" height="512" alt="glossy" src="https://github.com/user-attachments/assets/c62a2805-d9fa-47f4-af9a-0d8c28c7c4b8" />


<img width="512" height="512" alt="mirror no strat" src="https://github.com/user-attachments/assets/d0428aa0-8ad2-4be6-b8c6-d91742223dff" />
<img width="512" height="512" alt="mirror" src="https://github.com/user-attachments/assets/4ee0bc25-6014-440b-985d-316f06665e4b" />


<img width="512" height="512" alt="refraction no strat" src="https://github.com/user-attachments/assets/a6b69722-e08e-497a-b8e5-52eb2214249b" />
<img width="512" height="512" alt="refraction" src="https://github.com/user-attachments/assets/f21d300d-b741-4ac0-aa7f-e71ea0e72235" />


### Collaboration/References
Beer-Lambert: https://www.geeksforgeeks.org/physics/beer-lambert-law/

### Known Bugs
N/A

