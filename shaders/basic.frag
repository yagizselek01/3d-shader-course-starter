#version 330 core
in vec3 worldPosition;
in vec3 worldNormal;
 
uniform vec3 viewPosition;
uniform float time;
uniform int debugMode;
uniform int qualityLevel;

out vec4 FragColor; // The colour produced for this fragment.

const float FresnelPower = 2.4;

const float NoiseScale = 3.0;
const vec2 NoiseScrollSpeed = vec2(0.15, 0.08);
const vec2 NoiseSampleOffset = vec2(17.3, 9.2);
const float WarpStrength = 0.10;

const float MainLineFrequency = 22.0;
const float MainLineSpeed = 2.0;

const float EnergyLineFrequency = 25.0;
const float EnergyLineSpeed = 4.0;

const vec3 HologramColor =
    vec3(0.639, 0.110, 0.110);

const vec3 BrightColor =
    vec3(0.831, 0.447, 0.208);

// Hash function that deterministically maps 2D coordinates
// to a pseudo-random value in the range [0, 1].
float hash(vec2 p){
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}
// 2D value noise function.
// It generates smooth noise by hashing the surrounding grid corners
// and interpolating between those corner values.
float valueNoise(vec2 p)
{
    vec2 cell = floor(p); // Integer coordinates of the current grid cell
    vec2 local = fract(p); // Local position inside the cell, in the range [0, 1]

    // Hash the four corners of the current grid cell
    // to obtain pseudo-random scalar values.
    float a = hash(cell); 
    float b = hash(cell + vec2(1.0, 0.0));
    float c = hash(cell + vec2(0.0, 1.0));
    float d = hash(cell + vec2(1.0, 1.0));

    // Smooth the local coordinates using a cubic Hermite interpolation function
    vec2 smoothLocal =
        local * local * (3.0 - 2.0 * local);

    // Linearly interpolate along the bottom edge of the cell
    float bottom =
        mix(a, b, smoothLocal.x);

    // Linearly interpolate along the top edge of the cell
    float top =
        mix(c, d, smoothLocal.x);

    // Interpolate between the bottom and top results along Y.
    // This completes the two-dimensional interpolation using
    // the smoothed local coordinates.
    return mix(bottom, top, smoothLocal.y);
}

void main()
{
    // Normalize the interpolated world-space normal and construct the
    // fragment-to-camera view direction used by the Fresnel-style rim.
    vec3 N = normalize(worldNormal);
    vec3 V = normalize(viewPosition - worldPosition);


    // Stylized two-sided Fresnel term. abs() gives front/back symmetry;
    // clamp() guards against small floating-point overshoot after normalization.
    float fresnel = 1.0 - clamp(abs(dot(N, V)), 0.0, 1.0);

    float fresnelRim = pow(fresnel, FresnelPower);
    
    if (debugMode == 1)
    {
    FragColor = vec4(vec3(fresnel), 1.0);
    return;
    }

    if (debugMode == 2)
    {
    FragColor = vec4(vec3(fresnelRim), 1.0);
    return;
    }

    // Calculate the rim effect based on the fresnel effect
    float fieldIntensity = fresnelRim * 0.8 + 0.2;

    // Procedural scanlines and coordinate distortion

    vec2 pos = worldPosition.xy;

    // Simple quality uses the original coordinate domain.
    // Full quality can replace this with the noise-warped domain.
    vec2 warpedPos = pos;

    // Keep these available for debug visualization.
    float noiseX = 0.0;
    float noiseY = 0.0;

    // Noise is required by the full effect and by the debug modes
    // that explicitly visualize value noise or warped scanlines.
    bool finalUsesNoiseWarp = debugMode == 0 && qualityLevel == 1;

    bool needsNoise = finalUsesNoiseWarp || debugMode == 4 || debugMode == 6;

if (needsNoise)
{
    vec2 noiseCoord = pos * NoiseScale + time * NoiseScrollSpeed;

    noiseX = valueNoise(noiseCoord);

    noiseY = valueNoise(noiseCoord + NoiseSampleOffset);

    if (debugMode == 4)
    {
    FragColor = vec4(noiseX, noiseY, 0.0, 1.0);

    return;
    }

    vec2 noiseWarp = vec2(noiseX, noiseY) * 2.0 - 1.0;

    warpedPos = pos + noiseWarp * WarpStrength;
}
    
    vec2 effectPos = (debugMode == 7) ? pos : warpedPos;


    // coordinate jitter
    float jitterOffset =
    0.03 * sin(effectPos.x * 10.0 + time * 0.8) +
    0.01 * sin(effectPos.y * 10.0 + time * 0.4);

    // small noisy breakup
    float breakupNoise = hash(vec2(floor(pos.x * 30.0), 
    floor(pos.y * 30.0 + time * 6.0)));

    if (debugMode == 3)
    {
        FragColor = vec4(vec3(breakupNoise), 1.0);

        return;
    }

    // warp the y coordinates of the lines to create a dynamic effect
    float warpedYline01 = effectPos.y + jitterOffset + (breakupNoise - 0.5) * 0.5;

    vec2 bandP = mix(pos, effectPos, 0.6); // mix the original and warped coordinates for the energy bands

    if (debugMode == 5)
    {
        float unwarpedLine =sin(pos.y * MainLineFrequency - time * MainLineSpeed);

        float unwarpedLineMask = smoothstep(0.82, 1.0, unwarpedLine);

        FragColor = vec4(vec3(unwarpedLineMask), 1.0);

        return;
    }

    if (debugMode == 6)
    {
        float noiseWarpedLine = sin(warpedPos.y * MainLineFrequency - time * MainLineSpeed);

        float noiseWarpedLineMask = smoothstep(0.82, 1.0, noiseWarpedLine);

        FragColor = vec4(vec3(noiseWarpedLineMask), 1.0);

        return;
    }


    float warpedYline02 = bandP.y + 0.05 * sin(bandP.x * 8.0 + time * 2.0);

    float mainLine = sin(warpedYline01 * MainLineFrequency - time * MainLineSpeed) * 0.5 + 0.5;

    float energyLine = sin(warpedYline02 * EnergyLineFrequency - time * EnergyLineSpeed) * 0.5 + 0.5;

    // make the threshold vary slightly across the surface
    float threshold = 0.92 + breakupNoise * 0.06;

    // Create a separate hash-based mask that breaks up sections of the main scanlines.
    float lineBreakMask = smoothstep(0.25, 0.75,
    hash(vec2(floor(pos.x * 12.0), 
    floor(pos.y * 12.0 - time * 3.0))));

    float mainLines = smoothstep(threshold, 1.0, mainLine);
    float energyBands = smoothstep(0.80, 0.86, energyLine);

    mainLines *= lineBreakMask;

    // create a pulsing effect for the lines and energy bands
    float pulse = 0.8 + 0.2 * sin(time * 2.5);

    // create a main pulse effect for the overall alpha of the holographic sphere, 
    //which will make it appear to pulse in and out
    float mainPulse = 0.5 + 0.5 * sin(time * 1.5);

    // combine the main lines and energy bands to create a final line intensity value
    float lineIntensity = (mainLines * 0.16 + energyBands * 0.7) * (0.4 + 0.6 * pulse);

    // calculate the final brightness of the holographic sphere based on the rim effect and line intensity
    float brightness = 0.15 + fieldIntensity * 1.2 + lineIntensity;

    // mix the hologram color and bright color based on the line intensity, and multiply by the final brightness
    vec3 finalColor = mix(HologramColor, BrightColor, lineIntensity) * brightness;

    // calculate the final alpha value based on the rim effect and main pulse, to create a semi-transparent holographic appearance
    float alpha = 0.02 + fieldIntensity * 0.6 + mainPulse * 0.15; 

    //Debug Mode

    FragColor = vec4(finalColor, alpha);
}


//NOTES FOR REPORT DOCUMENT
/*
1. Sphere and Fresnel
The original cube was replaced with a procedurally generated sphere to make the view-dependent rim effect easier to observe. A sphere has continuously varying surface normals, 
producing a smooth change in the dot product between the world-space normal and the view direction. On the original cube, each flat face had a largely constant normal, causing the Fresnel response to change abruptly between faces.

In the fragment shader, I use a stylized Fresnel-inspired approximation based on 1 - |N · V|. 
The value increases as the viewing direction becomes more perpendicular to the surface normal, producing a stronger rim near the silhouette.

2. Two-sided Fresnel problem
An earlier implementation used 1.0 - max(dot(N, V), 0.0). Because the sphere is rendered from both sides, 
back-facing fragments can produce negative N · V values. Clamping these values to zero caused the resulting rim term to become exactly 1.0, 
making large parts of the back-facing surface fully bright.

I changed the calculation to use abs(dot(N, V)), creating a symmetric angular response for the two-sided force field. 
This is a stylized choice for the holographic effect rather than a physically complete Fresnel model.

3. Transparency and rendering order
Standard alpha blending is order-dependent, so the order in which transparent surfaces are rendered affects the final colour. 
For the convex sphere, I render the back faces first and the front faces second. Front-face culling is used during the first pass, 
followed by back-face culling during the second pass.

Depth testing remains enabled so the force field still respects previously rendered opaque geometry, 
while depth writes are disabled during the transparent passes using glDepthMask(GL_FALSE). This allows both the back and front surfaces of the transparent sphere to contribute to the final blended colour.

This two-pass method works well for the convex sphere used in this project, but it is not a general solution for complex or intersecting transparent geometry.

4. Procedural scanlines and distortion
The holographic surface combines the Fresnel-style rim with two animated procedural patterns: narrow main scanlines and broader energy bands. Both are generated using sine functions evaluated from world-space coordinates and time.

The main scanline coordinates are distorted using two sine-based offsets and a hash-generated pseudo-random value. The hash is evaluated on quantized coordinates using floor(), producing discrete spatial and temporal variation that gives the field a deliberately unstable, glitch-like appearance.

A second hash-based mask attenuates different sections of the main scanlines, reducing their uniformity. smoothstep() is used to shape the sine waves into controlled bands and lines.

The final RGB colour combines the view-dependent rim with the animated procedural line intensity. Alpha is controlled separately using the rim and a slower global pulse, producing a semi-transparent field whose opacity changes over time.

5. Value noise and domain warping
To extend the initial sine-based distortion, I implemented a 2D value-noise function. The input coordinate is divided into an integer grid cell using floor() and a local position inside that cell using fract(). A deterministic hash function generates pseudo-random scalar values at the four cell corners. The local coordinates are shaped with a cubic Hermite function, \(f(x)=x^2(3-2x)\), before the corner values are interpolated to produce a continuous noise field.

Two decorrelated samples of this noise field are used as the X and Y components of a two-dimensional displacement vector. The values are remapped from [0,1] to [-1,1], and the original world-space coordinate is displaced using warpedPos = pos + noiseWarp * warpStrength.

The important distinction is that the noise does not simply modify the final colour. Instead, it modifies the coordinate domain from which the procedural scanlines are evaluated. This causes the existing sine-based patterns to bend and vary organically while preserving the original mathematical structure of the scanline function.

I apply the warped domain selectively: the primary scanlines use the full warped coordinate, the broader energy bands use a partial interpolation between the original and warped coordinates, and the quantized hash-based glitch masks remain in the original domain.
*/
