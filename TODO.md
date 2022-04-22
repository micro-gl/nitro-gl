# TODO
3. plot sampling uvs = done
4. transform uvs of sampler_t = done
5. can make an optimized render node for rectangles: interleaved(one upload call) and constant ebo = DONE
6. maybe create a GVA - generic vertex attribute system = DONE
7. shader compositor_t = DONE 
8. general texture sampler_t = DONE
9. basic blending = done
10. basic compositing = done
11. test idea with backdrop sampling read = done
12. attach_shaders, update_shaders, ctors - DONE
13. static pool for main programs - DONE
14. murmur hashcode - DONE
15. compositing is a canvas state - DONE
16. circle sampler - DONE

17. static linear pool for samplers uniforms cache
18. rounded rect sampler
19. render node for triangle batches - wip
20. test idea of missing uvs coords for triangle batches
21. SDF shader with sampler
22. AA with rbos or filter shader
23. implement many samplers
24. gradients
25. investigate caching of uniforms per program

optimizations:
1. lazy back buffers_type. also, if taregt is requested as premul alpha,
   normal blending and any of the porter-duff, we can use opengl blending.

NOTES:
- all samplers should be linear space. If one is pre-mul like a texture,
then use an unmul sampler
- branch in vertex shader for made-up uvs for traingles if not present
- backdrop uvs should be calculated in place
- We can compute mip-maps at client side and the mip level at client side,
because, the mip level is uniform accros because it is 2d and aligned except for
quadrilaterals where there is a twist and the sampling rate is varying.
  https://www.reddit.com/r/opengl/comments/3cdg5r/derivation_of_opengls_mipmap_level_computation/
- all textures should be pre-multiplied alpha
- all other samplers should be un-multiplied
- 
- backdrop texture does not need interpolation and min filter, therefore it can be mul/unmul alpha
- if your texture uses any filter/interpolation and it has transperency, you need it to be alpha-mul
- canvas inherits the alpha-stat of it's texture
- opengl textures are top=1.0, bottom=0.0 

OPTIM:
- if normal blend-mode and any porter-duff and canvas is pre-mul stat, then you can skip backdrop texture and do it all in open-gl