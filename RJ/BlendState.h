#pragma once

class BlendState
{
public:

	/**
	* The BlendOperation defines how the final output pixel will be determined.
	* The Source (s) value is the pixel that is generated from the pixel shader.
	* The Destination (d) value is the value that is currently stored in the backbuffer.
	* Blend operations can be defined for the color and alpha values separately.
	* The Source and Destination values are first scaled by the source and
	* destination blend factors before the blend operation is applied.
	*/
	enum class BlendOperation // could also be called BlendOperand
	{
		Add,                // Add source and destination ( s + d ).
		Subtract,           // Subtract source from destination ( d - s )
		ReverseSubtract,    // Subtract destination from source ( s - s )
		Min,                // The minimum of source and destination ( min( s, d ) )
		Max                 // The maximum of source and destination ( max( s, d ) )
	};

	/**
	* The BlendFactor is used to scale the pixel operand (either Source (s) or
	* Destination (d) pixel value) before the blend operation is applied.
	*/
	enum class BlendFactor
	{
		Zero,               // Multiply the pixel operand by 0 before applying the blend operation.
		One,                // Multiply the pixel operand by 1 before applying the blend operation.
		SrcColor,           // Multiply the pixel operand by the source color (Cs) before applying the blend operation.
		OneMinusSrcColor,   // Multiply the pixel operand by ( 1 - Cs ) before applying the blend operation.
		DstColor,           // Multiply the pixel operand by the destination color (Cd) before applying the blend operation.
		OneMinusDstColor,   // Multiply the pixel operand by ( 1 - Cd ) before applying the blend operation.
		SrcAlpha,           // Multiply the pixel operand by the source alpha ( As ) before applying the blend operation.
		OneMinusSrcAlpha,   // Multiply the pixel operand by ( 1 - As ) before applying the blend operation.
		DstAlpha,           // Multiply the pixel operand by the destination alpha ( Ad ) before applying the blend operation.
		OneMinusDstAlpha,   // Multiply the pixel operand by ( 1 - Ad ) before applying the blend operation.
		SrcAlphaSat,        // Multiply the pixel operand by min( As, 1 - Ad ) before applying the blend operation.
		ConstBlendFactor,   // Multiply the pixel operand by a constant blend factor before applying the blend operation. See @BlendState::SetConstBlendFactor.
		OneMinusBlendFactor,// Multiply the pixel operand by ( 1 - blend factor ) before applying the blend operation. See @BlendState::SetBlendFactor.
		Src1Color,          // Multiply the pixel operand by the color of the 2nd output ( Cs1 ) from the pixel shader before applying the blend operation. @see https://msdn.microsoft.com/en-us/library/windows/desktop/bb205120(v=vs.85).aspx#Blending @see https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
		OneMinusSrc1Color,  // Multiply the pixel operand by ( 1 - Cs1 ) before applying the blend operation.
		Src1Alpha,          // Multiply the pixel operand by the alpha value of the 2nd output ( As1 ) from the pixel shader before applying the blend operation.
		OneMinusSrc1Alpha   // Multiply the pixel operand by ( 1 - As1 ) before applying the blend operation.
	};

	/**
	* Specify a logical pixel operation for rendering.
	* Supported since DirectX 11.1 and OpenGL 2.0+
	*/
	enum class LogicOperator
	{
		None,               // No logical operator is performed.
		Clear,              // Clears the render target (d = 0) at the current pixel.
		Set,                // Set the destination render target to 1 (d = 1) at the current pixel.
		Copy,               // Copy the source to the destination (d = s) at the current pixel.
		CopyInverted,       // Copy the inverted source to the destination (d = ~s) at the current pixel.
		Invert,             // Invert the color in the render target (d = ~d) at the current pixel.
		And,                // Logical And the source and destination values (d = s & d) at the current pixel.
		Nand,               // Logical !And the source and destination values (d = ~(s & d)) at the current pixel.
		Or,                 // Logical Or the source and destination values (d = s | d) at the current pixel.
		Nor,                // Logical !Or the source and destination values (d = ~( s | d) at the current pixel.
		Xor,                // Logical Xor the source and destination values (d = s ^ d) at the current pixel.
		Equiv,              // Logical !Xor at the source and destination values (d = ~(s ^ d) at the current pixel.
		AndReverse,         // Logical And the source and inverted destination values (d = s & ~d) at the current pixel.
		AndInverted,        // Logical And the inverted source  and destination values (d = ~s & d) at the current pixel.
		OrReverse,          // Logical Or the source and inverted destination values (d = s | ~d) at the current pixel.
		OrInverted          // Logical Or the inverted source and destination values (d = ~s | d) at the current pixel.
	};

	/**
	* A blend mode is used to define the blend function to apply to the source and
	* destination pixels before writing the final color to the back buffer.
	* is bound to the pixel output stage.
	* The BlendMode structure defines the blend function for one rendertarget.
	* If independent blending is enabled, then you can specify a different blend mode
	* for each color buffer that is bound to the current render target. If independent
	* blending is not enabled, then the BlendMode assigned to index 0 is used for
	* all bound render target color buffers.
	*/
	struct BlendMode
	{
		/**
		* Set to true to enable blending.
		* Default: false
		*/
		bool BlendEnabled;

		/**
		* Set to true to enable the logical operator.
		* Default: false
		*/
		bool LogicOpEnabled;

		/**
		* The blend factor to apply to the source color (S).
		* Default: BlendFactor::One
		*/
		BlendFactor SrcFactor;
		/**
		* The blend factor to apply the the destination color (D).
		* Default: BlendFactor::Zero
		*/
		BlendFactor DstFactor;

		/**
		* The blend operation to perform on the color components.
		* Default: BlendOperation::Add
		*/
		BlendOperation BlendOp;

		/**
		* The blend factor to apply to the source alpha (As)
		* Default: BlendFactor::One
		*/
		BlendFactor SrcAlphaFactor;

		/**
		* The blend factor to apply to the destination alpha (Ad)
		* Default: BlendFactor::Zero
		*/
		BlendFactor DstAlphaFactor;

		/**
		* The blend operation to perform on the alpha components.
		* Default: BlendOperation::Add
		*/
		BlendOperation AlphaOp;

		/**
		* The logical operator to perform at the current pixel.
		* Default: LogicOperator::None.
		*/
		LogicOperator LogicOp;

		/**
		* Enable writing to the red channel of the back buffer.
		* Default: true
		*/
		bool WriteRed;

		/**
		* Enable writing to the green channel of the back buffer.
		* Default: true
		*/
		bool WriteGreen;

		/**
		* Enable writing to the blue channel of the back buffer.
		* Default: true
		*/
		bool WriteBlue;

		/**
		* Enable writing to the alpha channel of the back buffer.
		* Default: true
		*/
		bool WriteAlpha;

		explicit BlendMode(bool enabled = false,
			bool logicOpEnabled = false,
			BlendFactor srcFactor = BlendFactor::One,
			BlendFactor dstFactor = BlendFactor::Zero,
			BlendOperation blendOp = BlendOperation::Add,
			BlendFactor srcAlphaFactor = BlendFactor::One,
			BlendFactor dstAlphaFactor = BlendFactor::Zero,
			BlendOperation alphaOp = BlendOperation::Add,
			LogicOperator logicOp = LogicOperator::None,
			bool writeRed = true,
			bool writeGreen = true,
			bool writeBlue = true,
			bool writeAlpha = true)
			: BlendEnabled(enabled)
			, LogicOpEnabled(logicOpEnabled)
			, SrcFactor(srcFactor)
			, DstFactor(dstFactor)
			, BlendOp(blendOp)
			, SrcAlphaFactor(srcAlphaFactor)
			, DstAlphaFactor(dstAlphaFactor)
			, AlphaOp(alphaOp)
			, LogicOp(logicOp)
			, WriteRed(writeRed)
			, WriteGreen(writeGreen)
			, WriteBlue(writeBlue)
			, WriteAlpha(writeAlpha)
		{}
	};


	// Set of standard blend modes
	class BlendModes
	{
	public:

		static const BlendMode		NoBlend;
		static const BlendMode		AlphaBlend;
		static const BlendMode		AdditiveBlend;

	};

	protected:

};