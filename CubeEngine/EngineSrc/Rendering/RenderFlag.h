#ifndef TZW_RENDERSTATE_H
#define TZW_RENDERSTATE_H


namespace tzw {
namespace RenderFlag
{
enum class BufferTarget{
    IndexBuffer,
    VertexBuffer
};
enum class IndicesType{
	Lines,
    Triangles,
    TriangleStrip,
    Patches,
};

enum class TextureType
{
    Texture2D,
    TextureCubeMap,
};

enum class RenderFunction
{
    DepthTest,
    AlphaBlend
};

enum class DepthTestMethod
{
    Less,
    LessOrEqual
};

enum class WarpAddress
{
    Repeat,
    Clamp,
    ClampToEdge,
};

enum class BlendingEquation
{
    Add,
};

enum class BlendingFactor
{
    One,
    Zero,
    SrcAlpha,
    OneMinusSrcAlpha,
    ConstantAlpha
};

enum class RenderStage
{
	COMMON,
	TRANSPARENT,
	AFTER_DEPTH_CLEAR,
};
}
} // namespace tzw

#endif // TZW_RENDERSTATE_H
