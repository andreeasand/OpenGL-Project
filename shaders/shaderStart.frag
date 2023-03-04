#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec3 fragPos;
in vec4 fragPosLightSpace;
out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform mat3 lightDirMatrix;
uniform	mat3 normalMatrix;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//spotlight

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

};
PointLight pointLight, pointLight2;

uniform int bool_spotlight; 
float constant = 1.0f;
float linear = 0.09f;
float quadratic = 0.032f;

//fog
uniform int bool_fog;

vec3 ambient;
float ambientStrength = 0.5f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.7f;
float shininess = 64.0f;

float computeShadow(){
// transformăm poziția fragmentului în coordonatele normalizate ale spațiului luminii.
//Deoarece transformarea în spațiul luminii a fost făcută în vertex shader, rămâne să aplicăm diviziunea perspectivă pentru a obține 
//coordonatele normalizate
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//returnează poziția fragmentului curent în intervalul [-1,1]. Deoarece valorile adâncimii
//fragmentului sunt în intervalul [0,1], ar trebui să transformăm coordonatele normalizate în consecință
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
//Acum putem eșantiona adâncimea existentă în harta de adâncime folosind aceste coordonate, deoarece
//acestea corespund acum cu coordonatele NDC
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
//adancimea fragmentului curent
	float currentDepth = normalizedCoords.z;
	float bias = 0.005f;
//Dacă adâncimea fragmentului curent este mai mare decât valoarea
//din harta adâncimii, fragmentul curent este în umbră. Altfel, este iluminat
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
//modificam fragmentele în afara frustumului hărții de adâncime care au o valoare a adâncimii
//mai mare de 1,0
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	return shadow;
}

float computeFog()
{
//ceata exponentiala patratica
//ia în considerare reducerea intensității luminii în funcție de distanță.
// Factorul de atenuare care va fi folosit reprezintă densitatea de ceață, această densitate fiind
//constantă în toată scena.
 float fogDensity = 0.05f;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

vec3 ambientSum= vec3(0.0f);
vec3 diffuseSum= vec3(0.0f);
vec3 specularSum = vec3(0.0f);
void CalcPointLight(PointLight light, vec3 fNormal, vec3 fragPos, vec3 viewDir)
{
    	vec3 lightDir = normalize(light.position - fragPos);  //se calc directia luminii
	vec3 cameraPosEye = vec3(0.0f);   //observatorul este pus in origine
	//directia luminii nu este constanta intre fragmente si trebuie calculata diferit
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 halfVector = normalize(lightDirN + viewDirN);
	vec3 normalEye = normalize(normalMatrix * fNormal);	
    float distance    = length(light.position - fragPos);
	//folositpt a reduce intensitatea luminii punctiforme cu distanta
    float att = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));   
	ambient = att * ambientStrength * lightColor; 
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor; 

        //compute specular light
        
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = att * specularStrength * specCoeff * lightColor;

       ambientSum+= ambient;
       diffuseSum+= diffuse;
	specularSum += specular;
} 

void computeLightComponents()
{	
	vec3 cameraPosEye = vec3(0.0f); //in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN;
	if(bool_spotlight == 0)
	{
		lightDirN = normalize(lightDir);
	
		//compute view direction 
		vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
		//compute half vector
		vec3 halfVector = normalize(lightDirN + viewDirN);
			
		//compute ambient light
		//nu vine dintr-o anumita directie, e aprox luminii globale imprastiata in jurul scenei
		//Calculul său nu depinde de poziția spectatorului sau de direcția luminii
		ambient = ambientStrength * lightColor;   
		
		//compute diffuse light
 		//e imprastiata in toate directiile
		//nu depinde de poziția spectatorului, dar depinde de direcția luminii.
		//depinde de normala suprafeței (obiectele orientate spre lumina sunt mai luminoase) și de direcția sursei de lumină, dar nu și de direcția de vizualizare
		diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;  
		
		//compute specular light
		//depinde de pozitia observatorului
		//Calculul său necesită să se știe cât de apropiată este orientarea suprafeței de reflecția dintre
		//direcția luminii și ochi. Astfel, calculul luminii speculare depinde de normala suprafaței, de direcția luminii
		//și de direcția de vizionare.
		
		float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);   
		specular = specularStrength * specCoeff * lightColor;
	}
	else
	{
		vec3 viewD=vec3(0.0f)-fPosEye.xyz;
		CalcPointLight(pointLight,fNormal,fragPos,viewD);
		CalcPointLight(pointLight2,fNormal,fragPos,viewD);
	}		
}

void main() 
{
	

	pointLight.position=vec3(5.42f,4.21f,-3.09f);
	pointLight.constant=1.0f;
	pointLight.linear=0.7f;   //si 0.7 era ok din tabel
	pointLight.quadratic=1.8f;  //si 1.8 era ok

	pointLight2.position=vec3(4.79f,4.27f,3.19f);
	pointLight2.constant=1.0f;
	pointLight2.linear=0.7f;   //si 0.7 era ok din tabel
	pointLight2.quadratic=1.8f;  //si 1.8 era ok

	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	vec3 color;
if(bool_spotlight ==0){
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	float shadow;
	if(bool_spotlight == 0)
		shadow = computeShadow();
	else
		shadow = 0.0f;
		
	 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
}
else
{
	ambientSum *= texture(diffuseTexture, fTexCoords).rgb;
	diffuseSum *= texture(diffuseTexture, fTexCoords).rgb;
	specularSum *= texture(specularTexture, fTexCoords).rgb;

	float shadow;
	if(bool_spotlight == 0)
		shadow = computeShadow();
	else
		shadow = 0.0f;
		
	 color = min((ambientSum + (1.0f - shadow) * diffuseSum) + (1.0f - shadow) * specularSum, 1.0f);
}	
	float fogFactor;
	vec4 fogColor;
	vec4 lastColor;
	if(bool_fog == 1)
	{
		fogFactor = computeFog();
		fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
		lastColor = fogColor * (1 - fogFactor) + vec4(color, 1.0f) * fogFactor;
	}
	else
	{
		lastColor = vec4(color, 1.0f);
	}
	
	fColor = lastColor;

}