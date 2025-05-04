# Patches statuses

Legend:

- :grey_exclamation: - not meant to upstream, for compatibility with GCC only
- :x: - not upstreamed
- :grey_question: - sent but not merged yet
- :arrow_up_small:  - upstreamed
- :arrow_down_small:  - backported

-----

- `"0001-LLVM-Cygwin-Fix-symbol-visibility-definition.patch"` :x:
- `"0002-LLVM-Cygwin-Remove-special-case-for-CXX-extensions-o.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/497fbd0ee0c4c50d46b5b777cdcc3a532d1dcc91
- `"0003-LLVM-Cygwin-Fix-shared-library-name-136599.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/08efca9c2c2b9005a0390965a2bad9ef208e4db4
- `"0004-LLVM-Cygwin-Fix-Signals-compatibility-with-Cygwin-AP.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/8a84a19cf9ccbc1d0878f51b0466dc3a3b93dbe3
- `"0005-LLVM-TargetParser-Handle-msys-targets-the-same-as-cy.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/6900e9026516963ae625b28dded2cdf0bd16e590
- `"0006-LLVM-Cygwin-Define-_GNU_SOURCE-on-Cygwin-as-well.-13.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/9633f87e34dddce49619e7fc2d75c659c61a9db1
- `"0102-Clang-Cygwin-Enable-few-conditions-that-are-shared-w.patch"` :x:
- `"0103-Clang-Cygwin-Enable-TLS.patch"` :x:
- `"0104-Clang-Cygwin-Fix-symbol-visibility-definition-138118.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/592243c1cb3ea53b34033132a87b0d14af9d1079
- `"0106-Clang-Cygwin-Disable-shared-libs-on-Cygwin-by-defaul.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/9bc20fbbadb18a1c3415d4442066ed35a6bcc175
- `"0107-Clang-Cygwin-Remove-erroneous-_WIN32-define-and-clea.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/abe93fe7c88c477343c884036982ddc15f820425
- `"0108-Clang-Cygwin-don-t-use-Bsymbolic-functions-138217.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/74d921c01ab4aefb5f7f14062ab4aef50d5c6413
- `"0109-hack-cygwin-allow-multiple-definition-c-index-test.patch"` :grey_exclamation:
- `"0110-Clang-Driver-add-a-Cygwin-ToolChain.patch"` :grey_question: https://github.com/llvm/llvm-project/pull/135691
- `"0111-Clang-Driver-use-__cxa_atexit-by-default-on-Cygwin.-.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/ca3a5d37ef64668234cbce7236dd640a98e2d687
- `"0112-hack-cygwin-define-_GLIBCXX_USE_CXX11_ABI-to-1-if-no.patch"` :grey_exclamation:
- `"0201-LLD-MinGW-Implement-dll-search-prefix-option.patch"` :x:
- `"0203-LLD-COFF-add-__-data-bss-_-start-end-__-symbols-for-.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/f66f2fe0e5be6dc6eb3ef4d42af8692f9adcdc80
- `"0204-LLD-MinGW-Fall-back-to-using-default-target-if-no-m-.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/e4a79b7122cbe6470c1a42e7b3549c37a1a6408d
- `"0205-LLD-COFF-Ensure-.bss-is-merged-at-the-end-of-a-secti.patch"` :arrow_up_small: https://github.com/llvm/llvm-project/commit/bb2f7596a8b963af06e9dde821dcea1252ba4892
