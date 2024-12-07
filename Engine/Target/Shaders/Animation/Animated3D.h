static const int MAX_BONES = 256;

struct Animated3D {
	column_major matrix boneMatrices[MAX_BONES];
};