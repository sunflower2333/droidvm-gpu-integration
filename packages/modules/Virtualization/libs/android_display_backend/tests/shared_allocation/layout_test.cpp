// Copyright 2026 The Android Open Source Project
// SPDX-License-Identifier: Apache-2.0
#include "shared_ahb_layout.h"
#include <cstdio>
#include <cstdlib>
#include <limits>

using namespace android_display;
unsigned checks = 0;
void expect(bool condition, const char* name) {
    ++checks;
    if (!condition) { std::fprintf(stderr, "FAIL %s\n", name); std::exit(1); }
}

int main() {
    SharedAhbLayoutFacts valid{.buffer_id = 42, .allocation_size = 12288, .modifier = 0,
        .width = 513, .height = 3, .native_stride = 528, .fourcc = kSharedAhbRgbaFourcc,
        .planes = 1, .offset = 4096, .stride = 2112, .sample_bits = 32,
        .plane_width = 513, .plane_height = 3, .plane_size = 8192,
        .subsample_x = 1, .subsample_y = 1};
    expect(validSharedAhbLayout(valid, kSharedAhbRgbaFourcc),
           "linear padded nonzero plane offset");
#define REJECT(field, value) do { auto bad = valid; bad.field = value; \
    expect(!validSharedAhbLayout(bad, kSharedAhbRgbaFourcc), #field " rejects " #value); } while (0)
    REJECT(buffer_id, 0); REJECT(width, 0); REJECT(height, 0);
    REJECT(width, 8193); REJECT(height, 8193); REJECT(fourcc, 0x34325241);
    REJECT(modifier, 1); REJECT(planes, 0); REJECT(planes, 2);
    REJECT(allocation_size, 0); REJECT(allocation_size, kSharedAhbMaxAllocation+1);
    REJECT(offset, -1); REJECT(offset, 12289); REJECT(offset, 4097);
    REJECT(stride, -1); REJECT(stride, 0); REJECT(stride, uint64_t{1}<<32);
    REJECT(stride, 2111); REJECT(native_stride, 527); REJECT(native_stride, 0);
    REJECT(sample_bits, 24); REJECT(plane_width, 512); REJECT(plane_height, 2);
    REJECT(plane_size, -1); REJECT(plane_size, 0); REJECT(plane_size, 8193);
    REJECT(plane_size, 2052+2112*2-1); REJECT(subsample_x, 2); REJECT(subsample_y, 0);
    REJECT(plane_size, std::numeric_limits<int64_t>::max());
    REJECT(offset, std::numeric_limits<int64_t>::max());
    auto exact = valid;
    exact.plane_size = 2052+2112*2;
    expect(validSharedAhbLayout(exact, kSharedAhbRgbaFourcc), "exact final visible byte fits");
    auto one = valid;
    one.height = 1; one.plane_height = 1; one.plane_size = 2052;
    expect(validSharedAhbLayout(one, kSharedAhbRgbaFourcc), "one row without padding mapped");
    auto bgra = valid;
    bgra.fourcc = kSharedAhbBgraFourcc;
    expect(validSharedAhbLayout(bgra, kSharedAhbBgraFourcc), "BGRA matches requested AR24");
    expect(!validSharedAhbLayout(bgra, kSharedAhbRgbaFourcc), "BGRA rejects AB24 request");
    expect(sharedAhbStorageHeight(0) == 0, "reject zero logical height");
    expect(sharedAhbStorageHeight(1) == 4, "one row needs four storage rows");
    expect(sharedAhbStorageHeight(3) == 4 && sharedAhbStorageHeight(4) == 4 &&
           sharedAhbStorageHeight(5) == 8, "four row boundary");
    expect(sharedAhbStorageHeight(1287) == 1288, "captured Edge height");
    expect(sharedAhbStorageHeight(8191) == 8192 && sharedAhbStorageHeight(8192) == 8192,
           "maximum height remains representable");
    expect(sharedAhbStorageHeight(8193) == 0 && sharedAhbStorageHeight(UINT32_MAX) == 0,
           "reject oversized height before round-up overflow");
    SharedAhbLayoutFacts storage{.buffer_id = 532, .allocation_size = 9232384, .modifier = 0,
        .width = 1774, .height = 1288, .native_stride = 1792, .fourcc = kSharedAhbBgraFourcc,
        .planes = 1, .offset = 0, .stride = 7168, .sample_bits = 32,
        .plane_width = 1774, .plane_height = 1288, .plane_size = 9232384,
        .subsample_x = 1, .subsample_y = 1};
    expect(validSharedAhbStorageLayout(storage, kSharedAhbBgraFourcc),
           "genuine storage has all 1288 full rows");
    auto shortPlane = storage;
    --shortPlane.plane_size;
    expect(validSharedAhbLayout(shortPlane, kSharedAhbBgraFourcc) &&
           !validSharedAhbStorageLayout(shortPlane, kSharedAhbBgraFourcc),
           "final visible byte is insufficient for full physical-row capacity");
    shortPlane = storage;
    shortPlane.allocation_size = shortPlane.plane_size = 9228288;
    expect(!validSharedAhbStorageLayout(shortPlane, kSharedAhbBgraFourcc),
           "captured old allocation cannot hold padded storage");
    auto unrounded = storage;
    unrounded.height = 1287;
    expect(!validSharedAhbStorageLayout(unrounded, kSharedAhbBgraFourcc),
           "storage height itself must be aligned");
    const SharedAhbContentExtent content{1774, 1287};
    expect(content.matchesStorage(1774, 1288) && !content.matchesStorage(1774, 1287) &&
           !content.matchesStorage(1774, 1292) && !content.matchesStorage(1775, 1288),
           "logical content authenticates exact physical extent");
    std::printf("RESULT passed=1 layout_checks=%u\n", checks);
}
