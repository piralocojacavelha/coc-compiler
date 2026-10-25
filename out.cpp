static auto coc_matchOnPair(auto coc_Left) {
return [=](auto coc_Right) { return (coc_matchOnExists(0)(0)); };
}

static auto coc_matchOnFile(auto coc_file) {
return [=](auto coc_Result) { return [=](auto coc_return) { return (coc_matchOnExists(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(coc_file)(0)([=](auto coc_Handle) { return [=](auto coc_handle_with_proof) { return (coc_matchOnExists(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(coc_handle_with_proof)(0)([=](auto coc_proof) { return [=](auto coc_handle) { return (coc_return(0)(0)(0)(0)(0)(0)(0)(0)(coc_proof)(coc_handle)); }; })); }; })); }; };
}

static auto coc_readEntireFile(auto coc_path) {
return (coc_matchOnMaybe(0)(0)(0)(0)(0)(0)(0)((coc_openFile(coc_path)))(0)([=](auto coc_file) { return (coc_matchOnExists(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(coc_file)(0)([=](auto coc_Handle) { return [=](auto coc_handle_with_proof) { return (coc_matchOnExists(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(coc_handle_with_proof)(0)([=](auto coc_proof) { return [=](auto coc_handle) { return (coc_readFile(0)(0)(0)(0)(0)(0)(0)(0)(coc_proof)(coc_handle)); }; })); }; })); })((coc_nothing(0))));
}

static auto coc_printContents(auto coc_path) {
return (coc_matchOnMaybe(0)((coc_matchOnMaybe(0)(0)(0)(0)(0)(0)(0)((coc_openFile(coc_path)))(0)([=](auto coc_file) { return (coc_matchOnExists(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(coc_file)(0)([=](auto coc_Handle) { return [=](auto coc_handle_with_proof) { return (coc_matchOnExists(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(coc_handle_with_proof)(0)([=](auto coc_proof) { return [=](auto coc_handle) { return (coc_readFile(0)(0)(0)(0)(0)(0)(0)(0)(coc_proof)(coc_handle)); }; })); }; })); })((coc_nothing(0)))))(0)(coc_printString)(coc_pure));
}

auto coc_main = []() { return (coc_matchOnMaybe(0)((coc_matchOnMaybe(0)(0)(0)(0)(0)(0)(0)((coc_openFile(coc_thisPath)))(0)([=](auto coc_file) { return (coc_matchOnExists(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(coc_file)(0)([=](auto coc_Handle) { return [=](auto coc_handle_with_proof) { return (coc_matchOnExists(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)(coc_handle_with_proof)(0)([=](auto coc_proof) { return [=](auto coc_handle) { return (coc_readFile(0)(0)(0)(0)(0)(0)(0)(0)(coc_proof)(coc_handle)); }; })); }; })); })((coc_nothing(0)))))(0)(coc_printString)(coc_pure)); }();

