# DroidVM GPU integration checkpoint

Current source components are pinned in [components.json](components.json), on the `work/gpu-checkpoint-20261108` branches of sunflower2333/crosvm, dxvk, vkd3d-proton and gunyah-guest-drivers-windows. This repository supplies the Android display backend overlay under its original AOSP path. Apply that directory to `packages/modules/Virtualization` in the Android build that provides crosvm's `crosvm_device_only` target. It contains the complete 76-file backend used by the current native AHB host.

## Shared AHB storage repair

A logical 1774 by 1287 shared BGRA buffer previously had stride 7168 and actual capacity 9228288 bytes. Vulkan requires complete storage rows through height 1288, or 9232384 bytes. The allocator now requests real AHB height rounded to four rows. It retains logical wire dimensions, real mapper and DMA-BUF capacity checks, logical SurfaceControl crop, and logical copy regions. Native Blt destination images import the physical AHB extent.

The exact frozen parent and candidate inventories and eight changed backend/test source hashes are retained under `provenance`. These inventories include other unmodified AOSP fixture/kernel files; those files are not copied into this small overlay repository. The corresponding crosvm checkpoint retains its exact 1850 source files separately.

## Validation status

- Local layout controls: 47 checks passed. Affected presenter async, lifetime, frame-rate and geometry controls passed. The full presenter suite has an inherited intermittent timing assertion reproduced on the untouched parent.
- Real Android RGBA and BGRA allocation: seven shapes and 93 checks per format passed. BGRA 1774 by 1287 now has authentic 9232384-byte backing at physical height 1288.
- Candidate host SHA256 `070fbc619b011b50a1c40e1486658e8dc17605c683f683f7f473bfe41d24050c` built successfully and was installed after normal Windows shutdown. The sole original VM started once with its original configuration, 6656 MiB and 8 vCPU. Fresh Windows KMD 58652 passive observation has zero resets.
- DXVK's 975 retained stage73 source inputs match the published tree. Ten ASAN/UBSAN process teardown host cases pass. VKD3D production allocator submission ownership and logical queue routing host controls pass.
- The original AIO64 DX12 depth initialization and bounded scene were validated earlier. The user reports the current x86 DX12 AIO also works. Full API conformance and long stability remain open.
- Post-fix 1774 by 1287 Windows shared-texture allocation, Native Blt odd-height copy/upload/readback, SurfaceFlinger crop, and Edge WebGL1/2 pixels remain pending. Source publication is not an Edge WebGL pass. Continuous 165 Hz cadence, zero-copy and performance acceptance also remain open.

Raw device logs, VM disk/firmware, installed programs, build outputs and rollback binaries remain outside this repository. Existing AOSP and DroidVM source license headers are preserved in each file.
