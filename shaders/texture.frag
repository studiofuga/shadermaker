/* texture.frag - texture lookup example */

//
// This defines a sampler.
// To use textures, load an image into a texture unit and
// assign the texture unit index to the sampler.
//
uniform sampler2D textureImage;


//
// entry point
//
void main( void )
{
	gl_FragColor = texture2D( textureImage, gl_TexCoord[0].st );
}
