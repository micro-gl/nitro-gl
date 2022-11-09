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
17. rounded rect_i sampler - DONE
18. masking sampler - DONE
19. draw mask - DONE
20. draw arc - DONE
21. draw pie - DONE
22. quadrilateral - DONE
23. line gradient - DONE
24. fast 2 colors gradient - DONE
25. circle gradient - DONE
26. angular gradient - DONE
27. capsule shape sampler - DONE
28. linear classifier sampler - DONE
29. test idea of missing uvs coords for triangle batches - DONE
30. render node for triangle batches - DONE
31. draw polygons - DONE
32. draw paths fills - DONE
33. draw stroke fills - Done
34. bezier patches - done
35. draw text, bitmap fonts - done
36. tint sampler - done
37. draw lines - done
38. test gldrawArrays for non-indexed - done
39. convex/fan dont need to allocate indices buffer - done
40. clean up other_functions from samplers - done
41. const_cast with fake const samplers to catch rval refs without overloading - done
42. add docs for canvas functions - done
43. compatibility with older than 3.0 opengl (attributes in/out, texture/2d etc...) - done
44. debug mode - done
45. glsl versioning - wip

46. create mix sampler
47. filter sampler
48. checkerboard sampler
49. https://stackoverflow.com/questions/327642/opengl-and-monochrome-texture
50. AA with rbos
51. convert refs to pointer template so we can forward besides pointer to multi sampler
53. text measure compute
54. draw text, sdf version
56. explore drawing 3d objects, might need new shader type and pool and render node

tests:
1. test canvas with injected texture


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