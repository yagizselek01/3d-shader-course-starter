#version 330 core
in vec3 worldPosition;
in vec3 worldNormal;
 
uniform vec3 viewPosition;
uniform float time;

out vec4 FragColor; // The colour produced for this fragment.

float hash(vec2 p) // Hash function to generate pseudo-random values based on input coordinates
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

void main()
{
    // Normalize the interpolated world-space normal and construct the
    // fragment-to-camera view direction used by the Fresnel-style rim.
    vec3 N = normalize(worldNormal);
    vec3 V = normalize(viewPosition - worldPosition);


    // Calculate the fresnel effect
    float fresnel = 1.0 - abs(dot(N, V));

    float fresnelRim = pow(fresnel, 2.4);

    // Calculate the rim effect based on the fresnel effect
    float fieldIntensity = fresnelRim * 0.8 + 0.2;
    
    // Procedural scanlines and coordinate distortion

    vec2 p = worldPosition.xy;

    // coordinate jitter
    float jitterOffset =
    0.03 * sin(p.x * 10.0 + time * 0.8) +
    0.01 * sin(p.y * 10.0 + time * 0.4);

    // small noisy breakup
    float breakupNoise =
    hash(vec2(floor(p.x * 30.0), floor(p.y * 30.0 + time * 6.0)));

    // warp the y coordinates of the lines to create a dynamic effect
    float warpedYline01 = p.y + jitterOffset + (breakupNoise - 0.5) * 0.5;

    float warpedYline02 = worldPosition.y + 0.05 * sin(worldPosition.x * 8.0 + time * 2.0);

    float line01 = sin(warpedYline01 * 72.0 - time * 8.0) * 0.5 + 0.5;

    float line02 = sin(warpedYline02 * 25.0 - time * 4.0) * 0.5 + 0.5;

    // make the threshold vary slightly across the surface
    float threshold =
    0.92 + breakupNoise * 0.06;

    // create a mask for the line breaks based on the breakup noise
    float lineBreakMask = smoothstep(0.25, 0.75, hash(vec2(floor(p.x * 12.0), floor(p.y * 12.0 - time * 3.0))));

    float mainLines = smoothstep(threshold, 1.0, line01);
    float energyBands = smoothstep(0.80, 0.86, line02);

    mainLines *= lineBreakMask;

    // create a pulsing effect for the lines and energy bands
    float pulse = 0.8 + 0.2 * sin(time * 2.5);

    // create a main pulse effect for the overall alpha of the holographic sphere, 
    //which will make it appear to pulse in and out
    float mainPulse = 0.5 + 0.5 * sin(time * 1.5);

    // combine the main lines and energy bands to create a final line intensity value
    float lineIntensity = (mainLines * 0.16 + energyBands * 0.7) * (0.4 + 0.6 * pulse);

    //main color of the holographic sphere and the bright color for the lines and energy bands
    vec3 hologramColor = vec3(0.639, 0.110, 0.110);
    vec3 brightColor = vec3(0.831, 0.447, 0.208);

    // calculate the final brightness of the holographic sphere based on the rim effect and line intensity
    float brightness = 0.15 + fieldIntensity * 1.2 + lineIntensity;

    // mix the hologram color and bright color based on the line intensity, and multiply by the final brightness
    vec3 finalColor = mix(hologramColor, brightColor, lineIntensity) * brightness;

    // calculate the final alpha value based on the rim effect and main pulse, to create a semi-transparent holographic appearance
    float alpha = 0.02 + fieldIntensity * 0.6 + mainPulse * 0.15; 

    // set the final color and alpha value for the fragment
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
*/