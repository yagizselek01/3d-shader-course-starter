#version 330 core

// Rasterization generates fragments for the covered samples of the cube.
// This shader uses interpolated surface data to produce a colour for each fragment.

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uv;

// lightDirection points from the surface toward the directional light.
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 viewPosition;
uniform vec3 baseColor;
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform sampler2D surfaceTexture;

out vec4 FragColor; // The colour produced for this fragment.

void main()
{
/*
    vec4 texel = texture(surfaceTexture, uv);
    vec3 materialColor = texel.rgb * baseColor;

    // Interpolation can change a normal's length, so normalize per fragment.
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(lightDirection);

    float diffuse = max(dot(N, L), 0.0);

    vec3 V = normalize(viewPosition - worldPosition);
    vec3 H = normalize(L + V);

    // Only a surface facing the light may receive a specular highlight.
    float specular = 0.0;
    if (diffuse > 0.0)
    {
        specular = pow(max(dot(N, H), 0.0), shininess);
    }

    vec3 ambientColor = ambientStrength * materialColor * lightColor;
    vec3 diffuseColor = diffuse * materialColor * lightColor;
    vec3 specularColor = specularStrength * specular * lightColor;

    vec3 color = ambientColor + diffuseColor + specularColor;
    FragColor = vec4(color, 1.0);

    */
    // For fersnel effect, we need to calculate the view direction and the normal direction
    vec3 N = normalize(worldNormal);
    vec3 V = normalize(viewPosition - worldPosition);


    // Calculate the fersnel effect
    float fersnel = pow(1.0 - abs(dot(N, V)), 2.4) * 0.8 + 0.2;

    vec3 hologramColor = vec3(0.988, 0.306, 0.306);

    vec3 finalColor = fersnel * hologramColor;
    float alpha = 0.15 + fersnel * 0.40; // Adjust the alpha value based on the fersnel effect

    FragColor = vec4(finalColor, alpha);
}


//NOTES FOR REPORT DOCUMENT
/*
1. Changing the shape cube to sphere was done to enhance the holographic effect.
The fragment shader was modified to implement a holographic effect using the Fresnel effect, 
which simulates how light interacts with the surface of the sphere. The Fresnel effect was calculated based 
on the angle between the view direction and the surface normal, resulting in a color that changes based on the viewing angle. 
The final color and alpha value were adjusted to create a semi-transparent holographic appearance.
It represents much more better in a sphere than a cube, as the curvature of the sphere allows for more dynamic light interactions and reflections, enhancing the holographic effect.

2. There was a fersnel problem in the previous implementation, 
because I was using (1.0 - max(dot(N, V), 0.0)) instead of (1.0 - abs(dot(N, V))) to calculate the Fresnel effect.)
This caused the Fersnel effect to be max in the back of the sphere, because the dot product of the normal and view 
direction would be negative when the view direction is opposite to the normal, with the (1.0 - max(dot(N, V), 0.0)) equation
it's always 1.0.

3. To properly render the semi-transparent holographic sphere, I implemented the following code in main.cpp:
"
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Transparent object should not write into depth buffer
        glDepthMask(GL_FALSE);

        glEnable(GL_CULL_FACE);

        // 1. Render back faces first
        glCullFace(GL_FRONT);

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(sphere.indices.size()),
            GL_UNSIGNED_INT,
            nullptr
        );

        // 2. Render front faces afterwards
        glCullFace(GL_BACK);

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(sphere.indices.size()),
            GL_UNSIGNED_INT,
            nullptr
        );

        // Restore normal state
        glDepthMask(GL_TRUE);
"
It ensures that the back faces of the sphere are rendered first, followed by the front faces,
so that the semi-transparent effect is correctly displayed, allowing for proper blending of the colors and transparency.
If the back faces were rendered after the front faces, the front faces would occlude the back faces,
resulting in an incorrect visual representation of the holographic effect. By rendering the back faces first, 
the blending of colors and transparency is preserved, creating a more realistic and visually appealing holographic sphere.
The problem was weird shading and tringular artifacts on the sphere, which was caused by the incorrect rendering order of the back and front faces.
Culling the front faces first and then rendering the back faces allowed for proper blending and eliminated the artifacts, resulting in a smooth and visually accurate holographic effect on the sphere.

*/