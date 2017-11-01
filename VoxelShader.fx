/*__     _______  _______ _       ____  _   _    _    ____  _____ ____
  \ \   / / _ \ \/ / ____| |     / ___|| | | |  / \  |  _ \| ____|  _ \
   \ \ / / | | \  /|  _| | |     \___ \| |_| | / ^ \ | | | |  _| | |_) |
    \ v /| |_| /  \| |___| |___   ___) |  _  |/ ___ \| |_| | |___|  _ <
	 \_/  \___/_/\_\_____|_____| |____/|_| |_/_/   \_\____/|_____|_| \_\

  VoxelShader.fx
  Author: Bart Kelchtermans
  Date: August, 2016
*/

// IMPORTANT NOTE: When importing 3D models it's important to enable 'Flip UV Coordinates' for textures to properly work.

// STATIC INPUT ________________________________________________________
cbuffer inputWorld
{
	float4x4 matWorld : WORLD;
	float4x4 matWorldViewProj : WORLDVIEWPROJECTION;
}

cbuffer inputScene
{
	float4x4 matViewInv : VIEWINVERSE;
	float4 gLightDirection : DIRECTION
	<
		string UIName = "Point Light Pos";
		string Object = "PointLight";
		string Space = "World";
	> = {100.0f, 200.0f, 1.0f, 1.0f};
}

// VARIABLES ___________________________________________________________
float gBlockSize
<
	string UIName = "Block Size";
	string UIWidget = "Slider";
	float UIMin = 2;
	float UIMax = 10;
	float UIStep = 1;
> = 5;

// STATES ______________________________________________________________
RasterizerState gRS_BackCulling
{
    CullMode = BACK;
};

BlendState gBS_DisableBlending
{
    BlendEnable[0] = FALSE;
};

SamplerState gTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// Wrap / Mirror / Clamp / Border
    AddressV = Wrap;// Wrap / Mirror / Clamp / Border
};

// DIFFUSE _____________________________________________________________
Texture2D gTextureDiffuse;

bool gUseDiffuseTexture
<
	string UIName = "Diffuse Texture";
	string UIWidget = "Bool";
> = false;

float3 gColorDiffuse
<
	string UIName = "Diffuse Color";
	string UIWidget = "Color";
> = float3(0.3,0.3,0.3);

// SPECULAR ____________________________________________________________
float3 gColorSpecular
<
	string UIName = "Specular Color";
	string UIWidget = "Color";
> = float3(1,1,1);

int gShininess
<
	string UIName = "Shininess";
	string UIWidget = "Slider";
	float UIMin = 1;
	float UIMax = 20;
	float UIStep = 0.01;
> = 10;

// FUNCTIONS____________________________________________________________
float3 CalculateDiffuseIntensity(float3 color, float3 normal)
{
	float diffuseIntensity = saturate(dot(normal, -gLightDirection));
	color *= diffuseIntensity;

	return color;
}

float3 CalculateSpecularBlinn(float3 viewDirection, float3 normal)
{
	float3 specularColor = gColorSpecular;
    float3 halfVector = normalize(viewDirection + gLightDirection);
    float specularStrenght = saturate(dot(normal, - halfVector));
    specularStrenght = pow(specularStrenght, gShininess);
    specularColor *= specularStrenght;

	return specularColor;
}

// STRUCTS _____________________________________________________________
struct VS_DATA
{
  float3 Position : POSITION;
  float3 Normal :   NORMAL;
  float2 TexCoord : TEXCOORD;
};
struct GS_DATA
{
  float4 Position : SV_POSITION;
  float4 WorldPosition: COLOR1;
  float3 Normal : NORMAL;
  float3 Color : COLOR;
};

// VERTEX SHADER _______________________________________________________
VS_DATA MainVS(VS_DATA vsData)
{
    return vsData;
}

// GEOMETRY SHADER _____________________________________________________
void AppendVertex(inout TriangleStream<GS_DATA> tStream, VS_DATA newVertex, float3 color, float3 offset)
{
  GS_DATA geomData;
  float4 offsetValue = float4(offset, 5);
  offsetValue.x /= 100;
  offsetValue.y /= 100;
  offsetValue.z /= 200;
  
  geomData.Position = mul(float4(newVertex.Position,1), matWorldViewProj) + offsetValue;
  geomData.WorldPosition = mul(float4(newVertex.Position,1), matWorld);
  geomData.Normal = mul(newVertex.Normal, (float3x3)matWorld);
  geomData.Color = color;
  tStream.Append(geomData);
}

[maxvertexcount(24)]
void Voxeliser(triangle VS_DATA vertices[3], inout TriangleStream<GS_DATA> tStream)
{		
	// Variables
	float3 center, offset;				// The clipped and original center of the triangle
	float distVertices[3]; 				// The distance between any of the vertices of the triangle
	float dist2; 						// The distance between the regular center and the clipped center
	float distSmallest, distLargest;	// Smallest / Largest distance between 2 vertices of the triangle 
	float distAvg;						// Average distance between the vertices of the triangle
	
	// Calculating the center of the given triangle
    center = vertices[0].Position + vertices[1].Position + vertices[2].Position;
    center /= 3;

	// Setting an offset to the original triangle center - this value is unique for every triangle	
	offset = center/gBlockSize;
	offset *= gBlockSize;
	
	// Clipping the center to the 3D grid, calculating the clipping distance
    center = round(center/gBlockSize);
	center *= gBlockSize;
	dist2 = distance(offset, center);
	
	// Calculating the distance between the 3 points, calculating the average
	distVertices[0] = distance(vertices[0].Position, vertices[1].Position);
	distVertices[1] = distance(vertices[1].Position, vertices[2].Position);
	distVertices[2] = distance(vertices[0].Position, vertices[2].Position);
	distAvg = (distVertices[0] + distVertices[1] + distVertices[2]) / 3;
	
	// Calculating area of the triangle using Heron's Formula
	// https://en.wikipedia.org/wiki/Heron%27s_formula
	float semiPer = (distVertices[0] + distVertices[1] + distVertices[2]) / 2;
	float triSurface = sqrt(semiPer * (semiPer - distVertices[0]) * (semiPer - distVertices[1]) * (semiPer - distVertices[2]));
	float quadSurface = pow(gBlockSize, 2);
	
	// Optimisation: specify what geometry to neglect
	float trisInBlock = quadSurface / triSurface;
	if (dist2 > (distAvg * (trisInBlock / (trisInBlock * 0.7)))) return;
	
	// Calculating color
    float3 color = gColorDiffuse;
	
	if (gUseDiffuseTexture)
	{
		// Sampling the pixel color off of the texture, taking the average of the 3 vertices
		color = gTextureDiffuse.SampleLevel(gTextureSampler,vertices[0].TexCoord,0);
		color += gTextureDiffuse.SampleLevel(gTextureSampler,vertices[1].TexCoord,0);
		color += gTextureDiffuse.SampleLevel(gTextureSampler,vertices[2].TexCoord,0);
		color/=3;
	}

    VS_DATA A, B, C, D;

    //X - 1
    A.Position  = center + float3(-gBlockSize,+gBlockSize,-gBlockSize);
    B.Position  = center + float3(-gBlockSize,+gBlockSize,+gBlockSize);
    C.Position  = center + float3(-gBlockSize,-gBlockSize,-gBlockSize);
    D.Position  = center + float3(-gBlockSize,-gBlockSize,+gBlockSize);
    A.Normal    = float3(-1,0,0);
    B.Normal    = float3(-1,0,0);
    C.Normal    = float3(-1,0,0);
    D.Normal    = float3(-1,0,0);
    A.TexCoord = float2(0,0);
    B.TexCoord = float2(1,0);
    C.TexCoord = float2(0,1);
    D.TexCoord = float2(1,1);

    AppendVertex(tStream, A, color, offset);
    AppendVertex(tStream, B, color, offset);
    AppendVertex(tStream, C, color, offset);
    AppendVertex(tStream, D, color, offset);
    tStream.RestartStrip();

    //Y - 1
    A.Position  = center + float3(-gBlockSize,-gBlockSize,+gBlockSize);
    B.Position  = center + float3(+gBlockSize,-gBlockSize,+gBlockSize);
    C.Position  = center + float3(-gBlockSize,-gBlockSize,-gBlockSize);
    D.Position  = center + float3(+gBlockSize,-gBlockSize,-gBlockSize);
    A.Normal    = float3(0,-1,0);
    B.Normal    = float3(0,-1,0);
    C.Normal    = float3(0,-1,0);
    D.Normal    = float3(0,-1,0);
    AppendVertex(tStream, A, color, offset);
    AppendVertex(tStream, B, color, offset);
    AppendVertex(tStream, C, color, offset);
    AppendVertex(tStream, D, color, offset);
    tStream.RestartStrip();

    //Z - 1
    A.Position  = center + float3(+gBlockSize,+gBlockSize,-gBlockSize);
    B.Position  = center + float3(-gBlockSize,+gBlockSize,-gBlockSize);
    C.Position  = center + float3(+gBlockSize,-gBlockSize,-gBlockSize);
    D.Position  = center + float3(-gBlockSize,-gBlockSize,-gBlockSize);
    A.Normal    = float3(0,0,-1);
    B.Normal    = float3(0,0,-1);
    C.Normal    = float3(0,0,-1);
    D.Normal    = float3(0,0,-1);
    AppendVertex(tStream, A, color, offset);
    AppendVertex(tStream, B, color, offset);
    AppendVertex(tStream, C, color, offset);
    AppendVertex(tStream, D, color, offset);
    tStream.RestartStrip();

    //X + 1
    A.Position  = center + float3(+gBlockSize,+gBlockSize,+gBlockSize);
    B.Position  = center + float3(+gBlockSize,+gBlockSize,-gBlockSize);
    C.Position  = center + float3(+gBlockSize,-gBlockSize,+gBlockSize);
    D.Position  = center + float3(+gBlockSize,-gBlockSize,-gBlockSize);
    A.Normal    = float3(1,0,0);
    B.Normal    = float3(1,0,0);
    C.Normal    = float3(1,0,0);
    D.Normal    = float3(1,0,0);
    AppendVertex(tStream, A, color, offset);
    AppendVertex(tStream, B, color, offset);
    AppendVertex(tStream, C, color, offset);
    AppendVertex(tStream, D, color, offset);
    tStream.RestartStrip();

	//Y + 1
    A.Position  = center + float3(-gBlockSize,+gBlockSize,-gBlockSize);
    B.Position  = center + float3(+gBlockSize,+gBlockSize,-gBlockSize);
    C.Position  = center + float3(-gBlockSize,+gBlockSize,+gBlockSize);
    D.Position  = center + float3(+gBlockSize,+gBlockSize,+gBlockSize);
    A.Normal    = float3(0,1,0);
    B.Normal    = float3(0,1,0);
    C.Normal    = float3(0,1,0);
    D.Normal    = float3(0,1,0);
    AppendVertex(tStream, A, color, offset);
    AppendVertex(tStream, B, color, offset);
    AppendVertex(tStream, C, color, offset);
    AppendVertex(tStream, D, color, offset);
    tStream.RestartStrip();
	
    //Z + 1
    A.Position  = center + float3(-gBlockSize,+gBlockSize,+gBlockSize);
    B.Position  = center + float3(+gBlockSize,+gBlockSize,+gBlockSize);
    C.Position  = center + float3(-gBlockSize,-gBlockSize,+gBlockSize);
    D.Position  = center + float3(+gBlockSize,-gBlockSize,+gBlockSize);
    A.Normal    = float3(0,0,1);
    B.Normal    = float3(0,0,1);
    C.Normal    = float3(0,0,1);
    D.Normal    = float3(0,0,1);
    AppendVertex(tStream, A, color, offset);
    AppendVertex(tStream, B, color, offset);
    AppendVertex(tStream, C, color, offset);
    AppendVertex(tStream, D, color, offset);
    tStream.RestartStrip(); 
}

// PIXEL SHADER ________________________________________________________
float4 MainPS(GS_DATA input) : SV_TARGET
{
	// Normalise
	input.Normal = normalize(input.Normal);
    input.Normal=-normalize(input.Normal);

	// Diffuse
	float3 diffuseColor = CalculateDiffuseIntensity(input.Color, input.Normal);

	// Specularity - using the blinn model
	float3 viewDirection = normalize(input.WorldPosition.xyz - matViewInv[3].xyz);
	float3 specularColor = CalculateSpecularBlinn(viewDirection, input.Normal);
    diffuseColor+=specularColor;

    return float4(diffuseColor,1);
}

// TECHNIQUES __________________________________________________________
technique10 DefaultTechnique
{
  pass p0
  {
	  SetVertexShader(CompileShader(vs_4_0, MainVS()));
      SetGeometryShader(CompileShader(gs_4_0, Voxeliser()));
      SetPixelShader(CompileShader(ps_4_0, MainPS()));
      SetRasterizerState(gRS_BackCulling);
      SetBlendState(gBS_DisableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
  }
}
