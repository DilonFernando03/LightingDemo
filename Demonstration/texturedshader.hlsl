cbuffer ConstantBuffer
{
    matrix worldViewProjection;
    matrix world;
    
    //material light
    float4 materialColour;
    
    //diffuse colour
    float4 diffuseColour;
    
    //ambient light
    float4 ambientLightColour;
    
    //directional light
    float4 directionalLightColour;
    float4 directionalLightVector;
    
    //specular light
    float3 eyeposition;
    float specularPower;
    float4 specularPassedColour;
    
    //point light
    float4 pointLightColour;

};

Texture2D Texture;
SamplerState ss;

struct VertexIn
{
    float3 InputPosition : POSITION;
    float3 Normal : NORMAL;
    float2 TextureCoordinate : TEXTURECOORDINATE;
};

struct VertexOut
{
    float4 OutputPosition : SV_POSITION;
    float4 Normal : TEXCOORD0;
    float4 WorldPosition : TEXCOORD1;
    float2 TextureCoordinate : TEXTURECOORDINATE;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to homogeneous clip space.
    vout.OutputPosition = mul(worldViewProjection, float4(vin.InputPosition, 1.0f));

    // Normal and World Position being calculated for the pixel shader
    vout.Normal = mul(world, float4(vin.Normal, 0.0f));
    vout.WorldPosition = mul(world, float4(vin.InputPosition, 1.0f));
    vout.TextureCoordinate = vin.TextureCoordinate;
    return vout;
    
}

float4 PS(VertexOut pin) : SV_Target
{
    // Producing the diffuse component
    float4 Normal = pin.Normal;
    float4 directionalLightVector1 = normalize(directionalLightVector);
    float4 negatedDirectionalLightVector = -directionalLightVector1;
    float diff = saturate(dot(Normal, negatedDirectionalLightVector));
    float4 diffuse = diff * diffuseColour;
    
    // Calculating the ambient colour
    float4 ambient = 0.4 * ambientLightColour;

    // Calculating the specular colour
    float3 lightDir = normalize(directionalLightVector.xyz);
    float3 viewDir = normalize(eyeposition - pin.WorldPosition.xyz);
    float3 reflectDir = reflect(-lightDir, Normal.xyz);
    float specularColour = pow(max(dot(viewDir, reflectDir), 0), 30);

    // Applying the specular light colour
    float4 finalSpecularColor = specularPassedColour * specularPower * specularColour;

    //Calculating the point light
    float3 pointPosition = float3(0.0f, 0.0f, -5.0f);
    float3 pointlightDir = normalize(pointPosition - pin.WorldPosition.xyz);
    float pointDistance = length(pointlightDir);
    float pointAttenuation = 1.0 / (0 + 0.3 * pointDistance + 0.5 * (pointDistance * pointDistance));
    float pointDiffuse = saturate(dot(Normal.xyz, pointlightDir));
    float4 PointLight = 0.1 * pointLightColour * (pointDiffuse);
    
    float4 finalColour = saturate((diffuse + PointLight + finalSpecularColor) * materialColour );

    // returning the final colour multiplied with the texture for each pixel
    return finalColour * Texture.Sample(ss, pin.TextureCoordinate);
}