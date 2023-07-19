# Computer Graphics Laboratory

![CG LAB](./assets/main.png)

## Dependencies, Compile and Run

Please, run in any linux distribution

* Dependencies
    * OpenGL 3.0
    * GLFW


* Compile
```bash
    make
```

* Execute 
```bash
    ./main
```

## Functionalities

### Edit scenario on the fly
![Filter Options](./assets/edit_scenario.png)

### Add objects on the fly
![Filter Options](./assets/add_gif.gif)

### Filters
![Filter Options](./assets/filters.png)

#### Pre Median Filter
![Filter Options](./assets/pre_filter.png)
#### Pos Median Filter
![Filter Options](./assets/pos_filter.png)


### Load scenario as a JSON
![Load JSON](./assets/load_json.png)
JSON Format
```json
    {

        "background": [179, 204, 255],
        "image_width": 500,
        "aspect_ratio": [1, 1],

        "lookfrom": [13.0, 2.0, 3.0],
        "lookat": [0.0, 0.0, 0.0],
        "vup": [0.0, 1.0, 0.0],
        "vfov": 20.0,
        "aperture": 0.0,
        "dist_to_focus": 10.0,
        "time0": 0.0,
        "time1": 1.0,

        "samples" : 100,
        "depth": 50,

        "texture":[
            {"type": "image", "filename":"./texture_image/earthmap.jpg"},
            {"type": "checker", "color1":[255, 0, 0], "color2":[255, 255, 0]},
            {"type": "noise", "scale":4},
            {"type": "color", "color":[0, 255, 0]}
        ],

        "material": [
            {"type": "lambertian", "texture":0},
            {"type": "metal", "color":[0, 255, 0], "fuzz":0.0},
            {"type": "dieletric", "index_refraction":1.5},
            {"type": "light", "texture":3},
            {"type": "isotropic", "texture":2}
        ],

        "hittable":[
            {"type": "sphere", "center":[0, 0, 0], "radius": 2, "material": 0, "show": true},
            {"type": "moving_sphere", "center0":[0, 0, 0], "center1":[1, 1, -1], "time0": 0.0, "time1": 1.0, "radius": 2, "material": 0, "show": true},
            {"type": "box", "start":[0, 0, 0], "end":[1, 1, -1], "material": 0, "show": true},
            {"type": "constant_medium", "hittable":2, "density":1.0, "texture": 3, "show": true},
            {"type": "xy_rect", "x0":0.0, "x1":1.0, "y0": 3.0, "y1": 5.0, "k": -3.0, "material": 0, "show": true},
            {"type": "xz_rect", "x0":0.0, "x1":1.0, "z0": 3.0, "z1": 5.0, "k": -3.0, "material": 0, "show": true},
            {"type": "yz_rect", "y0":0.0, "y1":1.0, "z0": 3.0, "z1": 5.0, "k": -3.0, "material": 0, "show": true},
            {"type": "translate", "hittable":0, "displacement": [1.0, 2.0, 3.0], "show": true},
            {"type": "rotate_y", "hittable":0, "angle": 30.1, "show": true}
        ]
    }
```



