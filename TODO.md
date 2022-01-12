# TODO
3. plot sampling uvs = done
4. transform uvs of sampler_t = done
5. can make an optimized render node for rectangles: interleaved(one upload call) and constant ebo = DONE
6. maybe create a GVA - generic vertex attribute system = DONE
7. shader compositor = DONE
8. 
9. static pool
10. render node for triangle batches
11. test idea with backdrop sampling read
12. test idea of missing uvs coords for triangle batches
13. basic blending
14. basic compositing
15. copy shader and node
16. a simple texture sampler_t
17. clipping
18. general texture sampler_t
19. AA with rbos or filter shader
20. implement many samplers

optimizations:
1. lazy back buffers. also, if taregt is requested as premul alpha,
   normal blending and any of the porter-duff, we can use opengl blending.