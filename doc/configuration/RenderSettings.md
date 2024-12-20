RenderSettings (settings, striding, renderer, scalewidth, scalecontours, renderhint, rendertextforvectors)
=============================================

Back to [Configuration](./Configuration.md)

# settings

settings : `auto` / `precise` / `fast`

Controls behaviour of nearestneighbour rendering
(https://github.com/KNMI/adaguc-server/blob/master/adagucserverEC/CImgWarpNearestNeighbour.h).

- `auto`: Automatically selects precise or fast setting based on grid size. Grids larger than 700x700 pixels will be done with fast (smaller grids with precise).
- `precise`: Gridded files up to 1000x1000 pixels can be done with the precise renderer. This uses the GenericDataWarper
- `fast`: Grid larger than 1000x1000 should be rendered with the option fast.  This uses the AreaMapper
- For files over 4000x4000 pixels tiling is recommended.

```xml
<RenderSettings settings="precise"/>
```

# rendertextforvectors

```xml
<RenderSettings rendertextforvectors="true" />
```

When set to true, it will render text with windbarbs.

# striding

Controls how many grid cells are skipped. E.g. if set to 2, every other
(even) grid cell is used for reading.

```xml
<RenderSettings striding="1"/>
```

# renderer

At the moment GD can be forced

```xml
<RenderSettings renderer="gd" />
```

# renderhint

The renderhint provides a hint to the renderer on what type of styling to use to generate the image. Can be `discreteclasses` to use ShadeInterval configuration.

```xml
<RenderSettings renderhint="discreteclasses" />
```

For example to use discrete legend classes with the nearest neighbour or bilinear renderer do:

```xml
 <Style name="elevation_nl">

    <RenderSettings renderhint="discreteclasses"/>

    <RenderMethod>nearest,bilinear</RenderMethod>

    <NameMapping name="nearest" title="Height map nearest neighbour" abstract="Height map for AHN using nearest neighbour rendering" />
    <NameMapping name="bilinear" title="Height map bilinear" abstract="Height map for AHN using bilinear rendering" />

    <ShadeInterval min="-80" max="-7" fillcolor="#08387b" bgcolor="#FF000080" />
    <ShadeInterval min="-7" max="-6" fillcolor="#084584" />
    <ShadeInterval min="-6" max="-5" fillcolor="#105184" />
    <ShadeInterval min="-5" max="-4" fillcolor="#105D84" />
    <ShadeInterval min="-4" max="-3" fillcolor="#10698C" />
    <ShadeInterval min="-3" max="-2.5" fillcolor="#18758C" />
    <ShadeInterval min="-2.5" max="-2" fillcolor="#18828C" />
    <ShadeInterval min="-2" max="-1.5" fillcolor="#189294" />
    <ShadeInterval min="-1.5" max="-1" fillcolor="#219694" />
    <ShadeInterval min="-1" max="-0.5" fillcolor="#219E8C" />
    <ShadeInterval min="-0.5" max="0" fillcolor="#18A284" />
    <ShadeInterval min="0" max="0.5" fillcolor="#18AA7B" />
    <ShadeInterval min="0.5" max="1" fillcolor="#18B26B" />
    <ShadeInterval min="1" max="1.5" fillcolor="#10BA63" />
    <ShadeInterval min="1.5" max="2" fillcolor="#10BE52" />
    <ShadeInterval min="2" max="2.5" fillcolor="#08C742" />
    <ShadeInterval min="2.5" max="3" fillcolor="#08CF31" />
    <ShadeInterval min="3" max="3.5" fillcolor="#08CF31" />
    <ShadeInterval min="3.5" max="4" fillcolor="#00DB00" />
    <ShadeInterval min="4" max="4.5" fillcolor="#10DF00" />
    <ShadeInterval min="4.5" max="5" fillcolor="#29E300" />
    <ShadeInterval min="5" max="6" fillcolor="#42E700" />
    <ShadeInterval min="6" max="7" fillcolor="#63EB00" />
    <ShadeInterval min="7" max="8" fillcolor="#7BEF00" />
    <ShadeInterval min="8" max="9" fillcolor="#94F300" />
    <ShadeInterval min="9" max="10" fillcolor="#B5F700" />
    <ShadeInterval min="10" max="12" fillcolor="#CEF700" />
    <ShadeInterval min="12" max="14" fillcolor="#EFFF00" />
    <ShadeInterval min="14" max="16" fillcolor="#FFFB00" />
    <ShadeInterval min="16" max="18" fillcolor="#FFF300" />
    <ShadeInterval min="18" max="20" fillcolor="#FFE700" />
    <ShadeInterval min="20" max="25" fillcolor="#F7D700" />
    <ShadeInterval min="25" max="30" fillcolor="#F7CF08" />
    <ShadeInterval min="30" max="35" fillcolor="#F7C708" />
    <ShadeInterval min="35" max="40" fillcolor="#F7BE08" />
    <ShadeInterval min="40" max="45" fillcolor="#F7B610" />
    <ShadeInterval min="45" max="50" fillcolor="#EFA610" />
    <ShadeInterval min="50" max="60" fillcolor="#EFA210" />
    <ShadeInterval min="60" max="70" fillcolor="#EF9A10" />
    <ShadeInterval min="70" max="80" fillcolor="#E78E18" />
    <ShadeInterval min="80" max="90" fillcolor="#E78621" />
    <ShadeInterval min="90" max="100" fillcolor="#DE7921" />
    <ShadeInterval min="100" max="125" fillcolor="#D66D29" />
    <ShadeInterval min="125" max="150" fillcolor="#D66929" />
    <ShadeInterval min="150" max="175" fillcolor="#CE6131" />
    <ShadeInterval min="175" max="200" fillcolor="#CE5931" />
    <ShadeInterval min="200" max="250" fillcolor="#C67739" />
    <ShadeInterval min="250" max="300" fillcolor="#C65139" />
  </Style>
  ```

  The legend will look like:

<div style="background: white; width: 100px;">
<img src="../../tests/expectedoutputs/TestWMS/test_WMSGetLegendGraphic_NearestRenderWithShadeInterval.png"/>
</div>