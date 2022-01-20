# TODO
3. plot sampling uvs = done
4. transform uvs of sampler_t = done
5. can make an optimized render node for rectangles: interleaved(one upload call) and constant ebo = DONE
6. maybe create a GVA - generic vertex attribute system = DONE
7. shader compositor
8. 
9. static pool for programs
10. static linear pool for samplers uniforms cache
11. render node for triangle batches
12. test idea with backdrop sampling read
13. test idea of missing uvs coords for triangle batches
14. basic blending
15. basic compositing
16. copy shader and node
17. a simple texture sampler_t
18. clipping
19. general texture sampler_t
20. AA with rbos or filter shader
21. implement many samplers

optimizations:
1. lazy back buffers_type. also, if taregt is requested as premul alpha,
   normal blending and any of the porter-duff, we can use opengl blending.

NOTES:
- all samplers should be linear space. If one is pre-mul like a texture,
then use an unmul sampler
- branch in vertex shader for made-up uvs for traingles if not present
- backdrop uvs should be calculated in place