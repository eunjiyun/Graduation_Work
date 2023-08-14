<조작키>

이동 : WASD
가속 : SHIFT
점프 : SPACE
무기 변경 : Q (칼, 총, 주먹)
공격 : 우클릭
아이템 획득 : C
모션 블러 : B
전체 화면 모드 : CTRL
종료 : ESC

id 입력 후 enter 누르고 비밀번호를 입력



<기존 디버그 모드 오류 목록>

D3D12 ERROR: ID3D12CommandList::DrawIndexedInstanced: The render target format in slot 0 does not match that specified by the current pipeline state. (pipeline state = R8G8B8A8_UNORM, render target format = R32_FLOAT, RTV ID3D12Resource* = 0x000001B914EA31A0:'Unnamed ID3D12Resource Object') [ EXECUTION ERROR #613: RENDER_TARGET_FORMAT_MISMATCH_PIPELINE_STATE]
: GameFramework.cpp에서 m_pStage->OnPreRender(m_pd3dCommandList, m_pLights, m_pStage->m_pd3dCbvSrvDescriptorHeap, Monsters, Players); 이 함수를 호출하면 발생

D3D12 ERROR: ID3D12CommandList::DrawIndexedInstanced: The descriptor heap (0x00000197087A1540:'Unnamed ID3D12DescriptorHeap Object') containing handle 0x197086950c8 is different from currently set descriptor heap 0x000001970870B9D0:'Unnamed ID3D12DescriptorHeap Object'. [ EXECUTION ERROR #554: SET_DESCRIPTOR_HEAP_INVALID]
D3D12 ERROR: ID3D12CommandList::DrawInstanced: The descriptor heap (0x00000197087A1540:'Unnamed ID3D12DescriptorHeap Object') containing handle 0x19708695030 is different from currently set descriptor heap 0x000001970870B9D0:'Unnamed ID3D12DescriptorHeap Object'. [ EXECUTION ERROR #554: SET_DESCRIPTOR_HEAP_INVALID]
: blur logic 추가 후 발생

D3D12 ERROR: ID3D12Device::CreateGraphicsPipelineState: Vertex Shader - Pixel Shader linkage error: Signatures between stages are incompatible. Semantic 'TEXCOORD' is defined for mismatched hardware registers between the output stage and input stage. [ STATE_CREATION ERROR #660: CREATEGRAPHICSPIPELINESTATE_SHADER_LINKAGE_REGISTERINDEX]
:

D3D12 WARNING: ID3D12Device::CreateCommittedResource: Ignoring InitialState D3D12_RESOURCE_STATE_COPY_DEST. Buffers are effectively created in state D3D12_RESOURCE_STATE_COMMON. [ STATE_CREATION WARNING #1328: CREATERESOURCE_STATE_IGNORED]
: 오브젝트에 텍스쳐 매핑한 후 발생

D3D12 WARNING: ID3D12CommandList::DrawInstanced: Viewport: 0 is non-empty while the corresponding scissor rectangle is empty.  Nothing will be written to the render target when this viewport is selected.  In D3D12, scissor testing is always enabled. [ EXECUTION WARNING #695: DRAW_EMPTY_SCISSOR_RECTANGLE]
:







