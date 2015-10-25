#pragma once

#ifndef __ErrorCodesH__
#define __ErrorCodesH__

#define HandleErrors(expr,result) (result = expr); if (result != ErrorCodes::NoError) return result; 

typedef int Result;	

namespace ErrorCodes {

	// Generic error codes												(0-999)
	const Result		NoError											= 0;
	const Result		UnknownError									= 1;
	const Result		InvalidHwnd										= 2;
	const Result		CouldNotInitialiseDirect3D						= 3;
	const Result		CouldNotAllocateSufficientMemory				= 4;
	const Result		CouldNotInitialiseWindow						= 5;
	const Result		CouldNotInitialiseCentralLoggingComponent		= 6;
	const Result		InvalidParameters								= 7;
	const Result		UnknownDataNodeType								= 8;

	// DirectX error codes												(1000-1999)
	const Result		CannotCreateDirect3DDevice						= 1000;
	const Result		CannotCreateDXLocaliserComponent				= 1001;
	const Result		CannotCreateCameraComponent						= 1002;
	const Result		CannotCreateViewFrustrum						= 1003;
	const Result		CannotCreateTextRenderer						= 1004;
	const Result		CannotCreate2DRenderManager						= 1005;
	const Result		CannotCreateFXManager							= 1006;
	const Result		CannotCreateOverlayRenderer						= 1007;

	const Result		InvalidDXLevelPassedToLocaliser					= 1060;
	const Result		CouldNotDetermineSupportedDXFeatureLevels		= 1061;
	const Result		FailedToApplyDesiredD3DFeatureLevel				= 1062;

	const Result		CannotLoadMesh									= 1100;
	const Result		MeshOptimisationFailed							= 1101;
	const Result		CouldNotAllocateModelVertexArray				= 1102;
	const Result		CouldNotAllocateModelIndexArray					= 1103;
	const Result		CouldNotCreateModelVertexBuffer					= 1104;
	const Result		CouldNotCreateModelIndexBuffer					= 1105;
	const Result		CouldNotCreateTextureObject						= 1106;
	const Result		CouldNotOpenModelFile							= 1107;
	const Result		CouldNotAllocateModelDataStorage				= 1108;
	
	const Result		CouldNotCreateShaderFromTextureFile				= 1109;
	const Result		CouldNotCreateLightShader						= 1110;
	const Result		LightVertexShaderCompilationFailed				= 1111;
	const Result		LightVertexShaderFileMissing					= 1112;
	const Result		LightPixelShaderCompilationFailed				= 1113;
	const Result		LightPixelShaderFileMissing						= 1114;
	const Result		CannotCreateLightVertexShader					= 1115;
	const Result		CannotCreateLightPixelShader					= 1116;
	const Result		CannotCreateLightObject							= 1117;
	const Result		CouldNotCreateVertexShaderInputLayout			= 1118;
	const Result		CouldNotCreateVertexShaderSamplerState			= 1119;
	const Result		CouldNotCreateVertexShaderMatConstBuffer		= 1120;
	const Result		CouldNotCreatePixelShaderLightConstBuffer		= 1121;
	const Result		CouldNotObtainShaderBufferLock					= 1122;
	const Result		CouldNotObtainLightShaderBufferLock				= 1123;
	const Result		CouldNotCreateDXInterfaceFactory				= 1124;
	const Result		CouldNotCreatePrimaryAdapterInterface			= 1125;
	const Result		CouldNotEnumeratePrimaryAdapterOutputs			= 1126;
	const Result		CouldNotDetermineAdapterDisplayModeCount		= 1127;
	const Result		CouldNotAllocateAdapterDisplayModeStorage		= 1128;
	const Result		CouldNotEnumerateAdapterDisplayModes			= 1129;
	const Result		CouldNotDetermineAdapterDescription				= 1130;
	const Result		CouldNotConvertAndStoreAdapterDescData			= 1131;
	const Result		CouldNotCreateD3DDeviceAndSwapChain				= 1132;
	const Result		CouldNotObtainPointerToBackBuffer				= 1133;
	const Result		CouldNotCreateRenderTargetView					= 1134;
	const Result		CouldNotCreateDepthBufferTextture				= 1135;
	const Result		CouldNotCreateDepthStencilState					= 1136;
	const Result		CouldNotCreateDepthStencilView					= 1137;
	const Result		CouldNotCreateRasteriserState					= 1138;
	const Result		CouldNotInitialiseShaderToUnsupportedModel		= 1139;
	const Result		ParticleVertexShaderCompilationFailed			= 1140;
	const Result		ParticleVertexShaderFileMissing					= 1141;
	const Result		ParticlePixelShaderCompilationFailed			= 1142;
	const Result		ParticlePixelShaderFileMissing					= 1143;
	const Result		CannotCreateParticleVertexShader				= 1144;
	const Result		CannotCreateParticlePixelShader					= 1145;
	const Result		CouldNotCreateParticleShader					= 1146;
	const Result		CouldNotCreateAlphaEnabledBlendState			= 1147;
	const Result		CouldNotCreateAlphaDisabledBlendState			= 1148;
	const Result		TextureVertexShaderCompilationFailed			= 1149;
	const Result		TextureVertexShaderFileMissing					= 1150;
	const Result		TexturePixelShaderCompilationFailed				= 1151;
	const Result		TexturePixelShaderFileMissing					= 1152;
	const Result		CannotCreateTextureVertexShader					= 1153;
	const Result		CannotCreateTexturePixelShader					= 1154;
	const Result		CouldNotCreateVertexShaderConstantBuffer		= 1155;
	const Result		CouldNotCreatePixelShaderConstantBuffer			= 1156;
	const Result		CouldNotObtainShaderCustomBufferLock			= 1157;
	const Result		CouldNotCreateTextureShader						= 1158;
	const Result		CouldNotAllocateIndexBuffer						= 1159;
	const Result		CouldNotAllocateVertexBufferSysMem				= 1160;
	const Result		CouldNotCreateVertexBuffer						= 1161;
	const Result		CouldNotCreateIndexBuffer						= 1162;
	const Result		FontVertexShaderCompilationFailed				= 1163;
	const Result		FontVertexShaderFileMissing						= 1164;
	const Result		FontPixelShaderCompilationFailed				= 1165;
	const Result		FontPixelShaderFileMissing						= 1166;
	const Result		CannotCreateFontVertexShader					= 1167;
	const Result		CannotCreateFontPixelShader						= 1168;
	const Result		CouldNotObtainVertexBufferLock					= 1169;
	const Result		CouldNotCreateDepthDisabledStencilView			= 1170;
	const Result		CouldNotLoadTextureData							= 1171;
	const Result		CouldNotCreateShaderResourceViewFromTexture		= 1172;
	const Result		CouldNotCreateTexcubeShader						= 1173;
	const Result		VertexShaderCompilationFailed					= 1174;
	const Result		VertexShaderFileMissing							= 1175;
	const Result		PixelShaderCompilationFailed					= 1176;
	const Result		PixelShaderFileMissing							= 1177;
	const Result		CannotCreateVertexShader						= 1178;
	const Result		CannotCreatePixelShader							= 1179;
	const Result		CouldNotCreateVertexShaderEffectNoiseBuffer		= 1180;
	const Result		CannotCreatePixelShaderEffectDistortionBuffer	= 1181;
	const Result		CouldNotObtainShaderEffectNoiseBufferLock		= 1182;
	const Result		CouldNotObtainShaderEffectDistortionBufferLock	= 1183;
	const Result		CouldNotCreateFireShader						= 1184;
	const Result		CouldNotCreateTextureFromD3DResource			= 1185;
	const Result		CouldNotCreateAlphaEnabledAdditiveBlendState	= 1186;
	const Result		CouldNotInitialiseInstanceBuffer				= 1187;
	const Result		SkinnedVertexShaderCompilationFailed			= 1188;
	const Result		SkinnedVertexShaderFileMissing					= 1189;
	const Result		SkinnedPixelShaderCompilationFailed				= 1190;
	const Result		SkinnedPixelShaderFileMissing					= 1191;
	const Result		CannotCreateSkinnedVertexShader					= 1192;
	const Result		CannotCreateSkinnedPixelShader					= 1193;
	const Result		CouldNotCreateVertexShaderPerFrameConstBuffer	= 1194;
	const Result		CouldNotCreateVertexShaderPerObjectConstBuffer	= 1195;
	const Result		CouldNotCreateShaderBoneTransformConstBuffer	= 1196;
	const Result		CouldNotCreateVertexShaderPerSubsetConstBuffer	= 1197;
	const Result		CannotCreateSkinnedNormalMapShader				= 1198;
	const Result		CouldNotCreateLightFadeShader					= 1199;
	const Result		CouldNotObtainLightFadeShaderBufferLock			= 1200;
	const Result		LightFadePixelShaderFileMissing					= 1201;
	const Result		LightFadePixelShaderCompilationFailed			= 1202;
	const Result		CannotCreateLightFadePixelShader				= 1203;
	const Result		CouldNotCreateLightHighlightShader				= 1204;
	const Result		CouldNotObtainLightHighlightShaderBufferLock	= 1205;
	const Result		LightHighlightPixelShaderFileMissing			= 1206;
	const Result		LightHighlightPixelShaderCompilationFailed		= 1207;
	const Result		CannotCreateLightHighlightPixelShader			= 1208;
	const Result		LightHighlightFadeVertexShaderCompilationFailed	= 1209;
	const Result		LightHighlightFadeVertexShaderFileMissing		= 1210;
	const Result		LightHighlightFadePixelShaderCompilationFailed	= 1211;
	const Result		LightHighlightFadePixelShaderFileMissing		= 1212;
	const Result		CannotCreateLightHighlightFadeVertexShader		= 1213;
	const Result		CannotCreateLightHighlightFadePixelShader		= 1214;
	const Result		CouldNotObtainLightHighlightFadeShaderBufferLock= 1215;
	const Result		CouldNotCreateLightHighlightFadeShader			= 1216;
	const Result		CouldNotInitialiseModelWithInvalidStringParams	= 1217;
	const Result		FoundNoDisplayModesAvailableForPrimaryAdapter	= 1218;
	const Result		FoundNoSupportedDisplayModeForResolution		= 1219;
	const Result		ShaderManagerInitialisationFailedOnNullDevice	= 1220; 
	const Result		CannotLoadCompiledShaderWithNullInputFile		= 1221;
	const Result		CouldNotOpenCompiledShaderFile					= 1222;
	const Result		ShaderManagerCouldNotCreateVertexShader			= 1223;
	const Result		ShaderManagerCouldNotCreatePixelShader			= 1224;
	const Result		ShaderManagerCouldNotCreateGeometryShader		= 1225;
	const Result		ShaderManagerCannotCreateInputDescFromNullData	= 1226;
	const Result		CannotInitialiseVolLineShaderWithNullInput		= 1227;
	const Result		ErrorCreatingVolLineVertexShader				= 1228;
	const Result		ErrorCreatingVolLineGeometryShader				= 1229;
	const Result		ErrorCreatingVolLinePixelShader					= 1230;
	const Result		ShaderManagerCannotCreateShaderWithNullInput	= 1231;
	const Result		ShaderManagerCannotCreateInputLayoutWithoutDesc = 1232;
	const Result		ShaderManagerCouldNotCreateInputLayout			= 1233;
		
	const Result		CannotCreateParticleVertexBuffers				= 1500;
	const Result		CannotRemoveParticlesWithInvalidParameters		= 1501;
	const Result		CouldNotAllocateParticleIndexBuffer				= 1502;
	const Result		CouldNotCreateParticleVertexBuffer				= 1503;
	const Result		CouldNotCreateParticleIndexBuffer				= 1504;
	const Result		CouldNotAllocateParticleDataMemory				= 1505;
	const Result		CouldNotAllocateParticleVertexMemory			= 1506;
	const Result		CannotAddEmitterToParticleEngineWithNullData	= 1507;
	const Result		ParticleEmitterKeyAlreadyAssigned				= 1508;
	const Result		CannotRemoveParticleEmitterThatDoesNotExist		= 1509;
	const Result		ReceivedNullParticleEmitterKey					= 1510;
	const Result		CouldNotCreateParticleEngine					= 1511;
	const Result		CannotLinkAllRequiredShadersToParticleEngine	= 1512;
	
	const Result		CouldNotCreateEffectManager						= 1600;
	const Result		CannotLinkAllRequiredShadersToEffectManager		= 1601;
	

	// External data read/write errors									(2000-2199)
	const Result		FileDoesNotExist								= 2000;
	const Result		CannotOpenFile									= 2001;
	const Result		NullFilenamePointer								= 2002;

	// DirectInput errors												(2200-2399)
	const Result		InvalidDirectInputDevice						= 2200;
	const Result		CannotCreateDirectInputDevice					= 2201;
	const Result		CannotCreateGameInputDevice						= 2202;

	// XML reading, processing and writing errors						(2400-2599)
	const Result		CannotLoadXMLDocument							= 2400;
	const Result		CannotFindXMLRoot								= 2401;
	const Result		InvalidXMLRootNode								= 2402;
	const Result		NullPointerToXMLDocument						= 2403;
	const Result		NullPointerToRootXMLElement						= 2404;
	const Result		NullPointerToXMLElement							= 2405;
	const Result		CouldNotSaveXMLDocument							= 2406;
	const Result		ForceTerminatedInfiniteCircularFileIndices  	= 2407;		// Where file links formed an infinite circular loop
	
	// Data input errors												(2600-2799)
	const Result		ErrorsOccuredWhileLoadingMeshes					= 2600;
	const Result		CannotLoadMeshForNullObject						= 2601;
	const Result		InsufficientDataToLoadModel						= 2602;
	const Result		CannotLoadModelWhereDuplicateAlreadyExists		= 2603;
	const Result		CannotLoadModelWithInvalidClass					= 2604;
	const Result		CannotLoadSimpleShipDetailsWithDuplicateCode	= 2605;
	const Result		CannotLoadComplexShipDetailsWithDuplicateCode	= 2606;
	const Result		CannotLoadCSSectionDetailsWithDuplicateCode		= 2607;
	const Result		CannotLoadImage2DGroupWithNullParameters		= 2608;
	const Result		InsufficientDataToConstructImage2DGroup			= 2609;
	const Result		CannotLoadSectionElementsIntoShipWithNullData	= 2610;
	const Result		CannotLoadManagedUIControlDefWithoutParameters	= 2611;
	const Result		UIControlDefinitionFailedValidationOnLoad		= 2612;
	const Result		CannotLoadTileWithoutCodeSpecified				= 2613;
	const Result		CannotLoadTileWithInvalidDefinitionCode			= 2614;
	const Result		CannotLoadUIComponentGroupWithNullParameters	= 2615;
	const Result		CouldNotCreateNewShipTileFromXMLData			= 2616;
	const Result		CouldNotGenerateNewTileFromDefinition			= 2617;
	const Result		CannotGenerateTileWithoutValidParameters		= 2618;
	const Result		CannotGenerateTileFromDefWithUnknownClass		= 2619;
	const Result		CannotBuildTileWithInvalidPointer				= 2620;
	const Result		CouldNotAllocateSpaceForCompoundTileModel		= 2621;
	const Result		TileFailedHardStopRequirementsDuringGeneration	= 2622;
	const Result		TileGenerationFailedWithUnknownError			= 2623;
	const Result		CouldNotCompileNullTile							= 2624;
	const Result		UnknownErrorInCreatingTileFromDefinition		= 2625;
	const Result		ErrorSettingUnknownTileClassType				= 2626;
	const Result		InvalidParametersForLoadingCompoundTileModel	= 2627;	
	const Result		TileFailedHardStopRequirements					= 2628;
	const Result		CannotLoadComplexShipTileClassWithNullData		= 2629;
	const Result		CouldNotLoadTileClassWithInvalidClassType		= 2630;
	const Result		CouldNotLoadAllRequiredTileClassData			= 2631;
	const Result		CouldNotLoadTileClassWithDuplicateCode			= 2632;
	const Result		CannotLoadResourceWithoutRequiredParameters		= 2633;
	const Result		CannotLoadSkinnedModelWithNullParameters		= 2634;
	const Result		CannotLoadActorWithNullParameters				= 2635;
	const Result		CouldNotLoadSkinnedModelWithMissingParameters	= 2636;
	const Result		CouldNotLoadDuplicateSkinnedModel				= 2637;
	const Result		CouldNotLinkBaseActorToSkinnedModel				= 2638;
	const Result		CannotLoadActorBaseWithInsufficientData			= 2639;
	const Result		CouldNotLoadDuplicateActorBaseData				= 2640;
	const Result		SkinnedModelCreationFailedDuringLoad			= 2641;
	const Result		CannotLoadAttributeGenerationDataWithNullData	= 2642;
	const Result		CannotLoadInstanceOfNullCSSection				= 2643;
	const Result		CannotLoadCCSectionInstanceWithInvalidCode		= 2644;
	const Result		CannotLoadSimpleShipLoadoutWithDuplicateCode	= 2645;
	const Result		UnknownErrorInstantiatingCSSection				= 2646;
	const Result		CannotLoadTerrainDefinitionWithInvalidParams	= 2647;
	const Result		CouldNotLoadTerrainDefWithoutRequiredData		= 2648;
	const Result		CouldNotLoadDuplicateTerrainDefinition			= 2649;
	const Result		CannotLoadFactionFromNullData					= 2650;
	const Result		CouldNotLoadFactionWithoutAllRequiredData		= 2651;
	const Result		CouldNotAddNewLoadedFaction						= 2652;
	const Result		CannotLoadAttachmentWithNullParameters			= 2653;
	const Result		CannotLoadAttachmentConstraintWithNullParams	= 2654;
	const Result		CannotLoadArticulatedModelWithNullParameters	= 2655;
	const Result		InvalidComponentDataInArticulatedModelNode		= 2656;
	const Result		ArticulatedModelHasInvalidComponentCount		= 2657;
	const Result		CouldNotLoadNewArticulatedModel					= 2658;
	const Result		ArticulatedModelContainsInvalidComponentDef		= 2659;
	const Result		CannotLoadAttachmentDataForArticulatedModel		= 2660;
	const Result		CannotLoadUnlinkedArticulatedModel				= 2661;
	const Result		CannotStoreNewArticulatedModelWithSpecifiedCode = 2662;
	const Result		CannotLoadTurretDataWithInvalidParams			= 2663;
	const Result		AttributeDoesNotExist							= 2664;
	const Result		CannotLoadTurretObjectWithoutAllRequiredData	= 2665;
	const Result		CannotLoadProjectileLauncherWithInvalidParams	= 2666;
	const Result		CannotLoadProjectileDefWithInvalidParams		= 2667;
	const Result		CannotLoadProjectileLauncherWithoutRequiredData = 2668;
	const Result		CannotLoadProjectileDefWithoutRequiredData		= 2669;
	const Result		CannotLoadTurretLauncherBlock					= 2670;

	// Data output errors												(2800-2899)
	const Result		CannotSaveSimpleShipWithNullReferences			= 2801;
	const Result		CannotSaveComplexShipSectionWithNullReferences	= 2802;
	const Result		CannotSaveEngineWithNullReferences				= 2803;
	const Result		CannotSaveComplexShipElementWithNullReferences	= 2804;
	const Result		CannotSaveCSSectionDetailsWithNullReferences	= 2805;
	const Result		CannotSaveComplexShipWithNullReferences			= 2806;
	const Result		CouldNotUpdateComplexShipRegisterFile			= 2807;
	const Result		CannotSaveBoundingVolumeWithNullReferences		= 2808;
	const Result		CannotSaveHardpointWithNullReferences			= 2809;
	const Result		CannotSaveOBBWithNullReferences					= 2810;
	const Result		CannotSaveTerrainWithNullReferences				= 2811;

	// Generic object errors											(3000-3199)
	const Result		ObjectHasNoInternalCode							= 3000;

	// Ship errors														(3200-3499)
	const Result		ShipHasInvalidCategory							= 3200;
	const Result		NoShipSpecifiedForLoadout						= 3201;
	const Result		InvalidShipSpecifiedForLoadout					= 3202;
	const Result		ShipHasNoHullDetails							= 3203;

	// Loadout errors													(3500-3599)
	const Result		CannotAssignLoadoutToNullShip					= 3500;
	const Result		CannotAssignLoadoutToIncompatibleShipClass		= 3501;
	const Result		CannotAssignNullLoadoutToShip					= 3502;
	const Result		ShipHasNoDefaultLoadoutSpecified				= 3503;
	const Result		ShipLoadoutDoesNotExist							= 3504;
	const Result		ShipDefaultLoadoutDoesNotExist					= 3505;	
	const Result		InvalidParametersForShipLoadoutAssignment		= 3506;

	// Universe, system and region errors								(3600-3799)
	const Result		RegionTooSmallForSpecifiedUpdateThreshold		= 3600;
	const Result		CannotReinitialiseSystem						= 3601;
	const Result		CannotInitialiseSystemWithoutValidCode			= 3602;
	const Result		CannotInitialiseSystemWithInvalidSize			= 3603;
	const Result		CannotAddNullObjectToSystem						= 3604;
	const Result		ObjectAlreadyExistsInOtherSystem				= 3605;
	const Result		ObjectAlreadyExistsInOtherSpatialTree			= 3606;
	const Result		CannotRemoveNullObjectFromSystem				= 3607;
	const Result		CannotInitialiseSystemWithoutRequiredParams		= 3608;

	// User interface, font & text-rendering errors						(3800-3899)
	const Result		CannotAllocateFontSpacingBuffer					= 3800;
	const Result		CannotLoadFontDataFile							= 3801;
	const Result		CannotCreateFontTextureObject					= 3802;
	const Result		CannotCreateFontObject							= 3803;
	const Result		CannotCreateFontShader							= 3804;
	const Result		CannotCreateTextSentenceObject					= 3805;
	const Result		CannotCreateTextVertexArray						= 3806;
	const Result		CannotCreateTextIndexArray						= 3807;
	const Result		CannotCreateTextVertexBuffer					= 3808;
	const Result		CannotCreateTextIndexBuffer						= 3809;
	const Result		TextSentenceExceedsMaxAllocatedSize				= 3810;
	const Result		InvalidFontSpecifiedForTextConstruction			= 3811;
	const Result		FontDataNoLongerExistsForTextConstruction		= 3812;
	const Result		FontDataNoLongerExistsForTextRendering			= 3813;
	const Result		CannotInitialiseFontsWithoutTextManager			= 3814;
	const Result		CannotPerformUpdateOnNullSentencePointer		= 3815;
	const Result		CannotInitialiseUIWithoutGameEngineComponents	= 3816;
	const Result		CouldNotAllocateImage2DVertexArray				= 3817;
	const Result		CouldNotAllocateImage2DIndexArray				= 3818;
	const Result		CouldNotCreateImage2DVertexBuffer				= 3819;
	const Result		CouldNotCreateImage2DIndexBuffer				= 3820;
	const Result		CouldNotObtainImage2DBufferLock					= 3821;
	const Result		CouldNotAllocateImage2DTextureObject			= 3822;
	const Result		CouldNotCreateRenderGroupFromGameData			= 3823;
	const Result		CannotInitialiseUIRenderGroupAsNotLoaded		= 3824;
	const Result		MissingDataRequiredToInitialiseShipDesignerUI	= 3825;
	const Result		CouldNotCreateShipDesignerUIController			= 3826;
	const Result		CouldNotInitialiseShipDesignerGrid				= 3827;
	const Result		CouldNotAllocateImage2DRenderGroupVertexArray	= 3828;
	const Result		CouldNotAllocateImage2DRenderGroupIndexArray	= 3829;
	const Result		CouldNotCreateImage2DRenderGroupVertexBuffer	= 3830;
	const Result		CouldNotCreateImage2DRenderGroupIndexBuffer		= 3831;
	const Result		CouldNotObtainImage2DRenderGroupBufferLock		= 3832;
	const Result		CouldNotAllocateImage2DRenderGroupTextureObject	= 3833;
	const Result		CannotInitialiseTextBlockWithInvalidParameters	= 3834;
	const Result		CouldNotAllocateSpaceForTextBlockBuffer			= 3835;
	const Result		CannotCreateControlFromInvalidDefinition		= 3836;
	const Result		CouldNotCreateTextComponentForManagedControl	= 3837;
	const Result		CouldNotCreateModelBuilderUIController			= 3838;
	const Result		CannotGetHandleToModelViewerUIComponent			= 3839;
	const Result		CannotLoadModelWithInvalidCodeToViewer			= 3840;
	const Result		CannotLoadModelWithoutModelViewerEntity			= 3841;
	const Result		CannotInitialiseMLTBlockWithInvalidParameters	= 3842;
	const Result		CouldNotAllocateMemoryForMLTBlock				= 3843;
	const Result		CouldNotCreateLineWithinMLTBlock				= 3844;
	const Result		CannotInitialiseMLTBlockWithoutParent			= 3845;
	const Result		CouldNotCreateConsoleUIController				= 3846;
	const Result		CannotInitialiseAllRequiredConsoleComponents	= 3847;
	const Result		CannotLoadNullCollisionDataFile					= 3848;
	const Result		CollisionDataFileDoesNotExist					= 3849;
	const Result		CouldNotLoadCollisionDataFromFile				= 3850;
	const Result		CannotSaveNullCollisionDataFile					= 3851;

	// Space object errors												(3900-3999)
	const Result		ChildIsAlreadyAttachedToAnObject				= 3900;
	const Result		CannotRemoveAttachmentThatDoesNotExist			= 3901;
	const Result		CannotBreakAttachmentToNullObject				= 3902;
	const Result		ChildDoesNotHaveAttachmentToBreak				= 3903;
	const Result		CannotBreakAttachmentParentMismatch				= 3904;
	const Result		CannotBreakAttachmentChildMismatch				= 3905;
	
	// Complex ship errors												(4000-4399)
	const Result		NullComplexShipElementNodeProvided				= 4001;
	const Result		CannotLoadComplexShipElementForInvalidCoords	= 4002;
	const Result		NullComplexShipDetailsNodeProvided				= 4003;
	const Result		CouldNotAllocateStorageForCShipElementSpace		= 4004;
	const Result		CannotInitialiseElementSpaceWithInvalidParams	= 4005;
	const Result		EncounteredNullCSElementDuringInitialisation	= 4006;
	const Result		NullComplexShipSectionDetailsNodeProvided		= 4007;
	const Result		NullComplexShipSectionNodeProvided				= 4008;
	const Result		CannotConstructShipSectionWithoutValidDetails	= 4009;
	const Result		CannotRotateElementSpaceByInvalidParameter		= 4010;
	const Result		ErrorWhileAllocatingRotatedElementSpace			= 4011;
	const Result		CannotRotateNullElementSpace					= 4012;
	const Result		CannotLoadTileWithInvalidParameters				= 4013;
	const Result		NullComplexShipTileDefinitionNodeProvided		= 4014;
	const Result		CannotReallocateElementSpaceWithInvalidParams	= 4015;
	const Result		CannotAddInvalidShipSectionToShip				= 4016;
	const Result		InvalidSizeSpecifiedForCSTileDefinition			= 4017;
	const Result		NullComplexShipTileDefinitionClass				= 4018;
	const Result		UnknownComplexShipTileDefinitionClass			= 4019;
	const Result		CouldNotSetComplexShipTileDefinitionClass		= 4020;
	const Result		CouldNotLoadModelDataForTileDefinition			= 4021;
	const Result		CouldNotChangeCSDetailsToNullObject				= 4022;
	const Result		CannotCompileTileWithInvalidSize				= 4023;
	const Result		CannotCopyElementSpaceWithNullSourceOrTarget	= 4024;
	const Result		FailedToGenerateCopyOfElementSpace				= 4025;
	const Result		CannotCopyTileDataFromNullSource				= 4026;
	const Result		InvalidComplexShipTileDefinitionClass			= 4027;
	const Result		CouldNotCreateNewCSTileDefinition				= 4028;

	// Ship designer errors												(4400-4599)
	const Result		ShipDesignerCannotLoadInvalidShipCode			= 4400;
	const Result		ShipDesignerCouldNotLoadAndCopyShipDetails		= 4401;
	const Result		ShipDesignerCannotSaveInvalidShipCode			= 4402;
	const Result		ShipDesignerCannotCreateShipDataDirectory		= 4403;
	const Result		CouldNotInitialiseSDSectionView					= 4404;
	const Result		CouldNotInitialiseSDCorridorView				= 4405;
	const Result		CouldNotInitialiseSDTileView					= 4406;

	// Errors with core game data structures and processes				(4600-4799)
	const Result		CouldNotInitialiseSpatialPartitioningTree		= 4600;
	const Result		CouldNotAllocateBinaryHeap						= 4601;

	// Resource and item crafting related errors						(4800-4999)
	const Result		CannotLoadResourceFromNullData					= 4800;
	const Result		ResourceCompoundValuesCouldNotAllBeDetermined	= 4801;
	const Result		EncounteredRecursionLimitValuingResources		= 4802;
	const Result		UnreportedErrorCalculatingResourceCompoundValue	= 4803;

	// Skinned / actor models and animation errors						(5000-5299)
	const Result		CouldNotSetSkinnedModelVertexBuffer				= 5000;
	const Result		CouldNotSetSkinnedModelIndexBuffer				= 5001;
	const Result		CannotInitialiseActorWithInvalidModelCode		= 5002;

	// Pathfinding errors												(5300-5599)
	const Result		CannotGenerateNavNetworkForNullParentEntity		= 5300;
	const Result		CannotGenerateNavNetworkWithInvalidSize			= 5301;
	const Result		NoNavigationNodeDataToCreateNetwork				= 5302;
	const Result		NavNodeNetworkFailedToBuildBeforeConnection		= 5303;
	const Result		CouldNotAllocateSpaceForNavNetworkNodes			= 5304;
	const Result		InvalidPathfindingParameters					= 5305;
	const Result		PathDoesNotExist								= 5306;
	const Result		UnknownPathfindingError							= 5307;	

	// Game console errors												(5600-5999)
	const Result		CannotExecuteNullConsoleCommand					= 5600;
	const Result		UnknownConsoleCommand							= 5601;
	const Result		ObjectDoesNotExist								= 5602;
	const Result		SystemDoesNotExist								= 5603;
	const Result		ObjectIsNotEnvironment							= 5604;
	const Result		NoSpatialPartitioningTreeToRender				= 5605;

	// Turret simulation / controller errors							(6000-6099)
	const Result		TurretModelDoesNotContainRequiredModelTags		= 6000;
	const Result		CannotSetTurretLauncherDefinition				= 6001;

}

#endif




