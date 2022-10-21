#pragma once

/*
	TODO:
		- дефолтные рендерресы в рендерсистему
		- генератор геометрии в рендерсистему
		- в генераторе геометрии передавать видовую матрицу, а не камеру
		- рендерстейт
		- в сцену добавить варианты камер (то есть в рендере только набор данных, а в сцене управляющая типа от первого лица, полет и т.д.)
		- возможно функция вращения объекта в сторону другого - https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.cpp
		- сделать возможность установки атрибутов в вао по типу (vec2, vec3, mat4, etc) чтобы не нужно было руками высчитывать размеры и отступы. (старый способ сохранить). не забыть это добавить в инстансинг
*/

#pragma region Header

#if defined(_MSC_VER)
#	pragma warning(push, 0)
//#	pragma warning(disable : 4365)
#endif

#include <vector>
#include <string>

#define GLM_FORCE_INLINE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_XYZW_ONLY
#define GLM_FORCE_SILENT_WARNINGS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif

#pragma endregion

#pragma region Base
namespace base
{
	inline constexpr int Min(int a, int b) { return a < b ? a : b; }
	inline constexpr float Min(float a, float b) { return a < b ? a : b; }
	//template<typename T> inline constexpr T Min(const T& a, const T& b) { return a < b ? a : b; }

	inline constexpr int Max(int a, int b) { return a > b ? a : b; }
	inline constexpr float Max(float a, float b) { return a > b ? a : b; }
	//template<typename T> inline constexpr T Max(const T& a, const T& b) { return a > b ? a : b; }

	inline constexpr int Clamp(int a, int min, int max) { return Max(Min(a, max), min); }
	inline constexpr float Clamp(float a, float min, float max) { return Max(Min(a, max), min); }
	//template<typename T> inline constexpr T Clamp(const T& a, const T& min, const T& max) { return Max(Min(a, max), min); }

	template<typename T, typename F>
	inline constexpr T Lerp(T lower, T upper, F gradient)
	{
		return lower + (upper - lower) * Max(static_cast<F>(0), Min(gradient, static_cast<F>(1)));
	}

	// Implementation from "08/02/2015 Better array 'countof' implementation with C++ 11 (updated)" - https://www.g-truc.net/post-0708.html
	template<typename T, size_t N>
	[[nodiscard]] constexpr size_t Countof(T const (&)[N])
	{
		return N;
	}
}
#pragma endregion

#pragma region Core
namespace core
{
	// Logging

	struct LogCreateInfo
	{
		const char* FileName = "../Log.txt";
	};

	void LogPrint(const char* str);
	void LogWarning(const char* str);
	void LogError(const char* str);
}
#pragma endregion

#pragma region Math
namespace math
{
	// Returns true if axis-aligned bounding boxes intersect.
	inline bool IntersectAABB(glm::vec3 a_position, glm::vec3 a_size, glm::vec3 b_position, glm::vec3 b_size);

	// Returns true if axis-aligned bounding box and ray intersect.
	inline bool IntersectAABBRay(const glm::vec3 aabb_position, const glm::vec3 aabb_size, const glm::vec3 ray_origin, const glm::vec3 ray_direction, glm::vec3& point);
}
#pragma endregion

#pragma region Platform
namespace platform
{
	struct WindowCreateInfo
	{
		const char* Title = "Game";
		int Width = 1024;
		int Height = 768;

		bool Fullscreen = false;
		bool Resizable = true;
		bool Vsync = false;
	};

	int GetFrameBufferWidth();
	int GetFrameBufferHeight();
	float GetFrameBufferAspectRatio();

	// Keyboard keys (US keyboard layout)
	// NOTE: Use GetKeyPressed() to allow redefining required keys for alternative layouts
	enum KeyboardKey 
	{
		KEY_NULL = 0,        // Key: NULL, used for no key pressed
		// Alphanumeric keys
		KEY_APOSTROPHE = 39,       // Key: '
		KEY_COMMA = 44,       // Key: ,
		KEY_MINUS = 45,       // Key: -
		KEY_PERIOD = 46,       // Key: .
		KEY_SLASH = 47,       // Key: /
		KEY_ZERO = 48,       // Key: 0
		KEY_ONE = 49,       // Key: 1
		KEY_TWO = 50,       // Key: 2
		KEY_THREE = 51,       // Key: 3
		KEY_FOUR = 52,       // Key: 4
		KEY_FIVE = 53,       // Key: 5
		KEY_SIX = 54,       // Key: 6
		KEY_SEVEN = 55,       // Key: 7
		KEY_EIGHT = 56,       // Key: 8
		KEY_NINE = 57,       // Key: 9
		KEY_SEMICOLON = 59,       // Key: ;
		KEY_EQUAL = 61,       // Key: =
		KEY_A = 65,       // Key: A | a
		KEY_B = 66,       // Key: B | b
		KEY_C = 67,       // Key: C | c
		KEY_D = 68,       // Key: D | d
		KEY_E = 69,       // Key: E | e
		KEY_F = 70,       // Key: F | f
		KEY_G = 71,       // Key: G | g
		KEY_H = 72,       // Key: H | h
		KEY_I = 73,       // Key: I | i
		KEY_J = 74,       // Key: J | j
		KEY_K = 75,       // Key: K | k
		KEY_L = 76,       // Key: L | l
		KEY_M = 77,       // Key: M | m
		KEY_N = 78,       // Key: N | n
		KEY_O = 79,       // Key: O | o
		KEY_P = 80,       // Key: P | p
		KEY_Q = 81,       // Key: Q | q
		KEY_R = 82,       // Key: R | r
		KEY_S = 83,       // Key: S | s
		KEY_T = 84,       // Key: T | t
		KEY_U = 85,       // Key: U | u
		KEY_V = 86,       // Key: V | v
		KEY_W = 87,       // Key: W | w
		KEY_X = 88,       // Key: X | x
		KEY_Y = 89,       // Key: Y | y
		KEY_Z = 90,       // Key: Z | z
		KEY_LEFT_BRACKET = 91,       // Key: [
		KEY_BACKSLASH = 92,       // Key: '\'
		KEY_RIGHT_BRACKET = 93,       // Key: ]
		KEY_GRAVE = 96,       // Key: `
		// Function keys
		KEY_SPACE = 32,       // Key: Space
		KEY_ESCAPE = 256,      // Key: Esc
		KEY_ENTER = 257,      // Key: Enter
		KEY_TAB = 258,      // Key: Tab
		KEY_BACKSPACE = 259,      // Key: Backspace
		KEY_INSERT = 260,      // Key: Ins
		KEY_DELETE = 261,      // Key: Del
		KEY_RIGHT = 262,      // Key: Cursor right
		KEY_LEFT = 263,      // Key: Cursor left
		KEY_DOWN = 264,      // Key: Cursor down
		KEY_UP = 265,      // Key: Cursor up
		KEY_PAGE_UP = 266,      // Key: Page up
		KEY_PAGE_DOWN = 267,      // Key: Page down
		KEY_HOME = 268,      // Key: Home
		KEY_END = 269,      // Key: End
		KEY_CAPS_LOCK = 280,      // Key: Caps lock
		KEY_SCROLL_LOCK = 281,      // Key: Scroll down
		KEY_NUM_LOCK = 282,      // Key: Num lock
		KEY_PRINT_SCREEN = 283,      // Key: Print screen
		KEY_PAUSE = 284,      // Key: Pause
		KEY_F1 = 290,      // Key: F1
		KEY_F2 = 291,      // Key: F2
		KEY_F3 = 292,      // Key: F3
		KEY_F4 = 293,      // Key: F4
		KEY_F5 = 294,      // Key: F5
		KEY_F6 = 295,      // Key: F6
		KEY_F7 = 296,      // Key: F7
		KEY_F8 = 297,      // Key: F8
		KEY_F9 = 298,      // Key: F9
		KEY_F10 = 299,      // Key: F10
		KEY_F11 = 300,      // Key: F11
		KEY_F12 = 301,      // Key: F12
		KEY_LEFT_SHIFT = 340,      // Key: Shift left
		KEY_LEFT_CONTROL = 341,      // Key: Control left
		KEY_LEFT_ALT = 342,      // Key: Alt left
		KEY_LEFT_SUPER = 343,      // Key: Super left
		KEY_RIGHT_SHIFT = 344,      // Key: Shift right
		KEY_RIGHT_CONTROL = 345,      // Key: Control right
		KEY_RIGHT_ALT = 346,      // Key: Alt right
		KEY_RIGHT_SUPER = 347,      // Key: Super right
		KEY_KB_MENU = 348,      // Key: KB menu
		// Keypad keys
		KEY_KP_0 = 320,      // Key: Keypad 0
		KEY_KP_1 = 321,      // Key: Keypad 1
		KEY_KP_2 = 322,      // Key: Keypad 2
		KEY_KP_3 = 323,      // Key: Keypad 3
		KEY_KP_4 = 324,      // Key: Keypad 4
		KEY_KP_5 = 325,      // Key: Keypad 5
		KEY_KP_6 = 326,      // Key: Keypad 6
		KEY_KP_7 = 327,      // Key: Keypad 7
		KEY_KP_8 = 328,      // Key: Keypad 8
		KEY_KP_9 = 329,      // Key: Keypad 9
		KEY_KP_DECIMAL = 330,      // Key: Keypad .
		KEY_KP_DIVIDE = 331,      // Key: Keypad /
		KEY_KP_MULTIPLY = 332,      // Key: Keypad *
		KEY_KP_SUBTRACT = 333,      // Key: Keypad -
		KEY_KP_ADD = 334,      // Key: Keypad +
		KEY_KP_ENTER = 335,      // Key: Keypad Enter
		KEY_KP_EQUAL = 336,      // Key: Keypad =
		// Android key buttons
		KEY_BACK = 4,        // Key: Android back button
		KEY_MENU = 82,       // Key: Android menu button
		KEY_VOLUME_UP = 24,  // Key: Android volume up button
		KEY_VOLUME_DOWN = 25 // Key: Android volume down button
	};

	bool IsKeyPressed(int key);  // Check if a key has been pressed once
	bool IsKeyDown(int key);     // Check if a key is being pressed
	bool IsKeyReleased(int key); // Check if a key has been released once
	bool IsKeyUp(int key);       // Check if a key is NOT being pressed
	int GetKeyPressed();         // Get key pressed (keycode), call it multiple times for keys queued, returns 0 when the queue is empty
	int GetCharPressed();        // Get char pressed (unicode), call it multiple times for chars queued, returns 0 when the queue is empty

	bool IsMouseButtonPressed(int button);          // Check if a mouse button has been pressed once
	bool IsMouseButtonDown(int button);             // Check if a mouse button is being pressed
	bool IsMouseButtonReleased(int button);         // Check if a mouse button has been released once
	bool IsMouseButtonUp(int button);               // Check if a mouse button is NOT being pressed
	int GetMouseX();                                // Get mouse position X
	int GetMouseY();                                // Get mouse position Y
	glm::vec2 GetMousePosition();                   // Get mouse position XY
	glm::vec2 GetMouseDelta();                      // Get mouse delta between frames
	void SetMousePosition(int x, int y);            // Set mouse position XY
	void SetMouseOffset(int offsetX, int offsetY);  // Set mouse offset
	void SetMouseScale(float scaleX, float scaleY); // Set mouse scaling
	float GetMouseWheelMove();                      // Get mouse wheel movement for X or Y, whichever is larger
	glm::vec2 GetMouseWheelMoveV();                 // Get mouse wheel movement for both X and Y
	void SetMouseCursor(int cursor);                // Set mouse cursor

	void EnableCursor();                            // Enables cursor (unlock cursor)
	void DisableCursor();                           // Disables cursor (lock cursor)
	bool IsCursorOnScreen();                        // Check if cursor is on the screen

}
#pragma endregion

#pragma region Renderer
namespace renderer
{
	struct RendererCreateInfo
	{
		float PerspectiveFOV = 45.0f;
		float PerspectiveNear = 0.01f;
		float PerspectiveFar = 1000.0f;

		glm::vec3 ClearColor = { 0.4f, 0.6f, 1.0f };
	};

	enum class RenderResourceUsage
	{
		Static,
		Dynamic,
		Stream,
	};

	struct UniformVariable
	{
		UniformVariable() = default;
		UniformVariable(int newId) { id = newId; }
		bool IsValid() const { return id > -1; }
		int id = -1;
	};

	// TODO: юниформы хранящие свой тип данных (и статус изменения)

	class ShaderProgram
	{
	public:
		bool CreateFromMemories(const char* vertexShaderMemory, const char* fragmentShaderMemory);
		void Destroy();

		void Bind();

		static void UnBind();

		UniformVariable GetUniformVariable(const char* name);

		void SetUniform(UniformVariable var, int value);
		void SetUniform(UniformVariable var, float value);
		void SetUniform(UniformVariable var, float x, float y, float z);
		void SetUniform(UniformVariable var, const glm::vec2& v);
		void SetUniform(UniformVariable var, const glm::vec3& v);
		void SetUniform(UniformVariable var, const glm::vec4& v);
		void SetUniform(UniformVariable var, const glm::mat3& mat);
		void SetUniform(UniformVariable var, const glm::mat4& mat);

		void SetUniform(const char* name, int value);
		void SetUniform(const char* name, float value);
		void SetUniform(const char* name, float x, float y, float z);
		void SetUniform(const char* name, const glm::vec2& v);
		void SetUniform(const char* name, const glm::vec3& v);
		void SetUniform(const char* name, const glm::vec4& v);
		void SetUniform(const char* name, const glm::mat3& mat);
		void SetUniform(const char* name, const glm::mat4& mat);

		bool IsValid() const { return m_id > 0; }

	private:
		unsigned createShader(unsigned type, const char* source) const;

		unsigned m_id = 0;
	};

	enum class TextureMinFilter
	{
		Nearest,
		Linear,
		NearestMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapNearest,
		LinearMipmapLinear,
	};

	enum class TextureMagFilter
	{
		Nearest,
		Linear,
	};

	enum class TextureWrapping
	{
		Repeat,
		MirroredRepeat,
		Clamp,
	};

	enum class TexelsFormat
	{
		None = 0,
		R_U8,
		RG_U8,
		RGB_U8,
		RGBA_U8,
		Depth_U16,
		DepthStencil_U16,
		Depth_U24,
		DepthStencil_U24,
	};

	struct Texture2DLoaderInfo
	{
		RenderResourceUsage usage = RenderResourceUsage::Static;

		TextureMinFilter minFilter = TextureMinFilter::NearestMipmapNearest;
		TextureMagFilter magFilter = TextureMagFilter::Nearest;
		TextureWrapping wrapS = TextureWrapping::Repeat;
		TextureWrapping wrapT = TextureWrapping::Repeat;
		TextureWrapping wrapR = TextureWrapping::Repeat;

		const char* fileName = nullptr;
		bool verticallyFlip = true;
		bool mipmap = true;
	};

	struct Texture2DCreateInfo
	{
		Texture2DCreateInfo() = default;
		Texture2DCreateInfo(const Texture2DLoaderInfo& loaderInfo)
		{
			usage = loaderInfo.usage;
			minFilter = loaderInfo.minFilter;
			magFilter = loaderInfo.magFilter;
			wrapS = loaderInfo.wrapS;
			wrapT = loaderInfo.wrapT;
			wrapR = loaderInfo.wrapR;
			mipmap = loaderInfo.mipmap;
		}

		RenderResourceUsage usage = RenderResourceUsage::Static;

		TextureMinFilter minFilter = TextureMinFilter::NearestMipmapNearest;
		TextureMagFilter magFilter = TextureMagFilter::Nearest;
		TextureWrapping wrapS = TextureWrapping::Repeat;
		TextureWrapping wrapT = TextureWrapping::Repeat;
		TextureWrapping wrapR = TextureWrapping::Repeat;

		TexelsFormat format = TexelsFormat::RGBA_U8;
		uint16_t width = 1;
		uint16_t height = 1;
		uint16_t depth = 1;
		uint8_t* data = nullptr;
		unsigned mipMapCount = 1; // TODO: only compressed
		bool mipmap = true;
	};

	class Texture2D
	{
	public:
		bool CreateFromMemories(const Texture2DCreateInfo& createInfo);
		bool CreateFromFiles(const Texture2DLoaderInfo& loaderInfo);

		void Destroy();

		void Bind(unsigned slot = 0);

		static void UnBind(unsigned slot = 0);

		bool IsValid() const { return id > 0; }

		unsigned id = 0;
	};

	class VertexBuffer
	{
	public:
		bool Create(RenderResourceUsage usage, unsigned vertexCount, unsigned vertexSize, const void* data);
		void Destroy();

		void Update(unsigned offset, unsigned size, const void* data);

		void Bind() const;

		unsigned GetVertexCount() const { return m_vertexCount; }

		bool IsValid() const { return m_id > 0; }

	private:
		RenderResourceUsage m_usage = RenderResourceUsage::Static;
		unsigned m_id = 0;
		unsigned m_vertexCount = 0;
		unsigned m_vertexSize = 0;
	};

	class IndexBuffer
	{
	public:
		bool Create(RenderResourceUsage usage, unsigned indexCount, unsigned indexSize, const void* data);
		void Destroy();

		void Bind() const;

		unsigned GetIndexCount() const { return m_indexCount; }
		unsigned GetIndexSize() const { return m_indexSize; }

		bool IsValid() const { return m_id > 0; }

	private:
		RenderResourceUsage m_usage = RenderResourceUsage::Static;
		unsigned m_id = 0;
		unsigned m_indexCount = 0;
		unsigned m_indexSize = 0;
	};

	enum class VertexAttributeType
	{
		Float,
		Matrix4
	};

	struct VertexAttribute
	{
		unsigned size;       // ignore in instanceAttr
		VertexAttributeType type;
		bool normalized;
		unsigned stride;     // = sizeof Vertex, ignore in instanceAttr
		const void* pointer; // (void*)offsetof(Vertex, tex_coord)}, ignore in instanceAttr
	};

	enum class PrimitiveDraw
	{
		Lines,
		Triangles,
		Points,
	};

	class VertexArrayBuffer
	{
	public:
		bool Create(VertexBuffer* vbo, IndexBuffer* ibo, const std::vector<VertexAttribute>& attribs);
		bool Create(VertexBuffer* vbo, IndexBuffer* ibo, VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs, const std::vector<VertexAttribute>& instanceAttribs);
		bool Create(const std::vector<VertexBuffer*> vbo, IndexBuffer* ibo, const std::vector<VertexAttribute>& attribs);

		void Destroy();

		// есть два варианта инстансинга
		//	1) массив юниформ. Недостаток: кол-во юниформ ограничено и зависит от железа
		//		в этом случае в шейдере нужно создать массив юниформ, например uniform mat4 MVPs[200];
		//		обращение в коде шейдера: gl_Position = MVPs[gl_InstanceID] * vec4(aPos, 1.0);
		//		записать значения shader3.SetUniform(("MVPs[" + std::to_string(i) + "]").c_str(), MVP);
		//		в VAO в Draw() отрисовать нужное число model.Draw(200); 
		//	
		//	2) инстансбуффер. Позволит больше отрисовать.
		//		в шейдере прописываются атрибуты layout(location = 2) in mat4 instanceMatrix;
		//		работа: gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
		//		создается вершинный буфер с кол-вом инстансом - instancevb.Create(Static, amount, sizeof(glm::mat4), &modelMatrices[0]);
		//		в вао вызывается SetInstancedBuffer().
		//		в Draw можно указать кол-во инстансов (но это не обязательно)
		void SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs);


		void Draw(PrimitiveDraw primitive = PrimitiveDraw::Triangles, uint32_t instanceCount = 1);
		void DrawElementsBaseVertex(PrimitiveDraw primitive, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex);

		bool IsValid() const { return m_id > 0; }

		static void UnBind();

	private:
		unsigned m_id = 0;
		VertexBuffer* m_vbo = nullptr;
		VertexBuffer* m_instanceBuffer = nullptr;
		IndexBuffer* m_ibo = nullptr;
		unsigned m_attribsCount = 0;
		unsigned m_instancedAttribsCount = 0;

	};

	class FrameBuffer
	{
	public:
		bool Create(int width, int height);
		void Destroy();

		void Bind(const glm::vec3& color);

		void BindTextureBuffer();

		static void MainFrameBufferBind();

		bool IsValid() const { return m_id > 0 && m_texColorBuffer > 0 && m_rbo > 0; }
		const glm::mat4& GetProjectionMatrix() { return m_projectionMatrix; }

	private:
		unsigned m_id = 0;
		unsigned m_texColorBuffer = 0;
		unsigned m_rbo = 0;
		int m_width = 0;
		int m_height = 0;
		glm::mat4 m_projectionMatrix;
	};

	const glm::mat4& GetCurrentProjectionMatrix();

	struct Vertex_Pos2
	{
		glm::vec2 position;
	};

	struct Vertex_Pos2_TexCoord
	{
		glm::vec2 position;
		glm::vec2 texCoord;
	};

	struct Vertex_Pos2_Color
	{
		glm::vec2 position;
		glm::vec3 color;
	};

	struct Vertex_Pos3
	{
		glm::vec3 position;
	};

	struct Vertex_Pos3_TexCoord
	{
		bool operator==(const Vertex_Pos3_TexCoord& other) const { return position == other.position && texCoord == other.texCoord; }

		glm::vec3 position;
		glm::vec2 texCoord;
	};

	struct Vertex_Pos3_Normal_TexCoord
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;
	};

	template<typename T> std::vector<VertexAttribute> GetVertexAttributes();
	template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos2>();
	template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos2_TexCoord>();
	template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos2_Color>();
	template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos3>();
	template<> inline std::vector<VertexAttribute> GetVertexAttributes<Vertex_Pos3_TexCoord>();


	namespace TextureFileManager
	{
		Texture2D* LoadTexture2D(const char* name);
		Texture2D* LoadTexture2D(const Texture2DLoaderInfo& loaderInfo);
	}
}
#pragma endregion

#pragma region Graphics3D
namespace g3d
{
	class Camera
	{
	public:
		void MoveForward(float deltaTime, float speedMod = 1.0f);
		void MoveBackward(float deltaTime, float speedMod = 1.0f);
		void MoveRight(float deltaTime, float speedMod = 1.0f);
		void MoveLeft(float deltaTime, float speedMod = 1.0f);
		void MoveUp(float deltaTime, float speedMod = 1.0f);
		void MoveDown(float deltaTime, float speedMod = 1.0f);

		void Rotate(float offsetX, float offsetY);

		void SimpleMove(float deltaTime);
		void Update();

		void SetRotate(float yaw, float pitch);
		void SetPosition(const glm::vec3& pos) { m_position = pos; }
		void SetYaw(float val) { m_yaw = val; }
		void SetPitch(float val) { m_pitch = val; }
		void SetSpeed(float val) { m_movementSpeed = val; }
		void SetSensitivity(float val) { m_sensitivity = val; }

		float GetYaw() const { return m_yaw; }
		float GetPitch() const { return m_pitch; }

		const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
		const glm::vec3& GetPosition() const { return m_position; }
		const glm::vec3& GetDirection() const { return m_front; }
		const glm::vec3& GetRight() const { return m_right; }
		const float GetSpeed() const { return m_movementSpeed; }

		//Frustum ComputeFrustum() const;
		//private:
		void updateVectors();
	private:
		glm::mat4 m_viewMatrix;

		// camera Attributes
		glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_front = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_right;
		glm::vec3 m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		// euler Angles
		float m_yaw = -90.0f;
		float m_pitch = 0.0f;

		// camera options
		float m_movementSpeed = 5.0f;
		float m_sensitivity = 0.1f;
	};

	class Material
	{
	public:

	//private:
		renderer::Texture2D* diffuseTexture = nullptr;

		glm::vec3 ambientColor = glm::vec3(1.0f);
		glm::vec3 diffuseColor = glm::vec3(1.0f);
		glm::vec3 specularColor = glm::vec3(0.0f);
		float shininess = 1.0f;
	};
	
	class Model
	{
	public:
		bool Create(const char* fileName, const char* pathMaterialFiles = "./");
		bool Create(std::vector<renderer::Vertex_Pos3_TexCoord>&& vertices, std::vector<uint32_t>&& indices); // TODO: правильно?
		bool Create(const std::vector<renderer::Vertex_Pos3_TexCoord>& vertices, const std::vector<uint32_t>& indices);
		void Destroy();

		void SetInstancedBuffer(renderer::VertexBuffer* instanceBuffer, const std::vector<renderer::VertexAttribute>& attribs);

		void Draw(uint32_t instanceCount = 1);

		bool IsValid() const { return m_vertexBuffer.IsValid() && m_indexBuffer.IsValid() && m_vao.IsValid(); }
		const std::vector<renderer::Vertex_Pos3_TexCoord>& GetVertices() const { return m_vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_indices; }

	private:
		bool createBuffer();
		std::vector<renderer::Vertex_Pos3_TexCoord> m_vertices;
		std::vector<uint32_t> m_indices;

		Material m_material;

		renderer::VertexBuffer m_vertexBuffer;
		renderer::IndexBuffer m_indexBuffer;
		renderer::VertexArrayBuffer m_vao;
	};

	namespace ModelFileManager
	{
		Model* LoadModel(const char* name);
	}

	namespace drawPrimitive
	{
		void DrawLine(const Camera& camera, const glm::vec3& startPos, const glm::vec3& endPos);

		void DrawCubeWires(const Camera& camera, const glm::mat4& worldMatrix, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, bool disableDepthTest = false);
		void DrawCubeWires(const Camera& camera, const glm::vec3& position, const glm::vec3& size = glm::vec3(1.0f), const glm::vec3& rotationRadian = glm::vec3(0.0f), const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, bool disableDepthTest = false);
		inline void DrawCubeWires(const Camera& camera, const glm::vec3& position, const glm::vec4& color, bool disableDepthTest = false)
		{
			drawPrimitive::DrawCubeWires(camera, position, glm::vec3(1.0f), glm::vec3(0.0f), color, disableDepthTest);
		}
		inline void DrawCubeWires(const Camera& camera, const glm::vec3& position, const glm::vec3& size, const glm::vec4& color, bool disableDepthTest = false)
		{
			drawPrimitive::DrawCubeWires(camera, position, size, glm::vec3(0.0f), color, disableDepthTest);
		}
	}
}
#pragma endregion

#pragma region Graphics2D
namespace g2d
{
	class Font;

	class Text
	{
	public:
		bool Create(const std::string& fontFileName, uint32_t fontSize);
		void Destroy();

		void SetText(const std::wstring& text);
		void Draw(const glm::vec3& position, const glm::vec3& color, const glm::mat4& orthoMat);

	private:
		bool create(Font* font);

		std::wstring m_text;
		Font* m_font = nullptr;
		unsigned vao = 0;
		unsigned vertexBuffer = 0;
		unsigned indexBuffer = 0;
		uint16_t indexElementCount = 0;
		float angle = 0;
	};
}
#pragma endregion

#pragma region Scene
namespace scene
{
	class Transform
	{
	public:
		void Reset()
		{
			SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
			SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
			SetRotate(glm::vec3(0.0f, 0.0f, 0.0f));
		}

		void SetPosition(const glm::vec3& pos) { m_translation = pos; m_update = true; }
		void SetScale(const glm::vec3& scale) { m_scale = scale; m_update = true; }
		void SetRotate(const glm::vec3& radianAngle) { m_quaternion = glm::quat(radianAngle); m_update = true; }

		const glm::mat4& GetWorldMatrix();
	private:
		glm::vec3 m_translation = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::quat m_quaternion = glm::quat(glm::vec3(0.0f));
		glm::mat4 m_worldMatrix;
		bool m_update = true;
	};
}
#pragma endregion

#pragma region Physics
namespace physics
{

}
#pragma endregion

#pragma region World
namespace world
{

}
#pragma endregion

#pragma region Engine
namespace engine
{
	struct EngineCreateInfo
	{
		core::LogCreateInfo Log;
		platform::WindowCreateInfo Window;
		renderer::RendererCreateInfo Render;
	};

	bool CreateEngine(const EngineCreateInfo& createInfo);
	void DestroyEngine();

	bool IsRunningEngine();
	void BeginFrameEngine();
	void EndFrameEngine();

	float GetDeltaTime();
}
#pragma endregion

#include "Engine.inl"