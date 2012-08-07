1 stdcall -noname XWSAStartup(long long)
2 stdcall -noname XWSACleanup()
3 stub -noname @
4 stub -noname @
5 stub -noname @
6 stub -noname @
7 stub -noname @
8 stub -noname @
9 stub -noname @
10 stub -noname @
11 stub -noname @
12 stub -noname @
13 stub -noname @
14 stub -noname @
15 stub -noname @
16 stub -noname @
17 stub -noname @
18 stub -noname @
19 stub -noname @
20 stub -noname @
21 stub -noname @
22 stub -noname @
23 stub -noname @
24 stub -noname @
25 stub -noname @
26 stub -noname @
27 stub -noname @
28 stub -noname @
29 stub -noname @
30 stub -noname @
31 stub -noname @
32 stub -noname @
33 stub -noname @
34 stub -noname @
35 stub -noname @
37 stub -noname @
38 stdcall -noname NetDll_ntohs(long)
39 stub -noname @
40 stdcall -noname NetDll_htons(long)
51 stdcall -noname XNetStartup(long)
52 stub -noname @
53 stub -noname @
54 stub -noname @
55 stub -noname @
56 stub -noname @
57 stub -noname @
58 stub -noname @
60 stub -noname @
62 stub -noname @
63 stub -noname @
64 stdcall -noname XNetXnAddrToMachineId(long long)
65 stub -noname @
66 stub -noname @
67 stub -noname @
68 stub -noname @
69 stub -noname @
70 stub -noname @
71 stdcall -noname NetDll_XNetQosServiceLookup(long long long)
72 stub -noname @
73 stdcall -noname XNetGetTitleXnAddr(long)
75 stdcall -noname XNetGetEthernetLinkStatus()
76 stub -noname @
77 stub -noname @
78 stub -noname @
79 stub -noname @
81 stub -noname @
82 stub -noname @
83 stdcall -noname XNetGetSystemLinkPort(long)
84 stdcall -noname XNetSetSystemLinkPort(long)
472 stub -noname @
473 stub -noname @
474 stub -noname @
476 stub -noname @
477 stub -noname @
478 stub -noname @
479 stub -noname @
651 stdcall -noname XNotifyGetNext(long long long long)
652 stdcall -noname XLIVE_652(long)
653 stub -noname @
1082 stdcall -noname XGetOverlappedExtendedError(long)
1083 stdcall -noname XGetOverlappedResult(long long long)
5000 stdcall -noname XLIVE_5000(long)
5001 stdcall -noname XLIVE_5001(long)
5002 stdcall -noname XLIVE_5002()
5003 stdcall -noname XLiveUninitialize()
5005 stdcall -noname XLiveOnCreateDevice(long long)
5006 stub -noname @
5007 stdcall -noname XLiveOnResetDevice(long)
5008 stdcall -noname XHVCreateEngine(long long long)
5010 stub -noname @
5011 stub -noname @
5012 stub -noname @
5016 stdcall -noname XLivePBufferAllocate(long long)
5017 stdcall -noname XLivePBufferFree(long)
5018 stdcall -noname XLivePBufferGetByte(long long long)
5019 stdcall -noname XLivePBufferSetByte(long long long)
5020 stdcall -noname XLivePBufferGetDWORD(long long long)
5021 stdcall -noname XLivePBufferSetDWORD(long long long)
5022 stub -noname @
5023 stub -noname @
5024 stub -noname @
5025 stub -noname @
5026 stub -noname @
5027 stub -noname @
5028 stub -noname @
5029 stub -noname @
5030 stdcall -noname XLivePreTranslateMessage(long)
5031 stdcall -noname XLiveSetDebugLevel(long long)
5032 stub -noname @
5034 stdcall -noname XLiveProtectData(long long long long long)
5035 stdcall -noname XLiveUnprotectData(long long long long long)
5036 stdcall -noname XLiveCreateProtectedDataContext(long long)
5037 stdcall -noname XLiveQueryProtectedDataInformation(long long)
5038 stdcall -noname XLiveCloseProtectedDataContext(long)
5039 stub -noname @
5206 stub -noname @
5208 stub -noname @
5209 stub -noname @
5210 stub -noname @
5212 stub -noname @
5214 stub -noname @
5215 stub -noname @
5216 stub -noname @
5230 stub -noname @
5231 stub -noname @
5233 stub -noname @
5234 stub -noname @
5235 stub -noname @
5236 stub -noname @
5237 stub -noname @
5238 stub -noname @
5250 stub -noname @
5251 stdcall -noname XCloseHandle(long)
5252 stub -noname @
5254 stdcall -noname XLiveCancelOverlapped(long)
5255 stub -noname @
5256 stdcall -noname XEnumerate(long long long long long)
5257 stub -noname @
5258 stub -noname @
5259 stub -noname @
5260 stub -noname @
5261 stdcall -noname XUserGetXUID(long long)
5262 stdcall -noname XUserGetSigninState(long)
5263 stdcall -noname XUserGetName(long long long)
5264 stub -noname @
5265 stdcall -noname XLiveUserCheckPrivilege(long long long)
5266 stub -noname @
5267 stdcall -noname XUserGetSigninInfo(long long long)
5270 stdcall -noname XNotifyCreateListener(long long)
5271 stub -noname @
5273 stub -noname @
5274 stub -noname @
5275 stub -noname @
5276 stub -noname @
5277 stdcall -noname XUserSetContext(long long long)
5278 stdcall -noname XUserWriteAchievements(long long long)
5279 stub -noname @
5280 stdcall -noname XUserCreateAchievementEnumerator(long long long long long long long long long)
5281 stdcall -noname XUserReadStats(long long long long long long long long)
5282 stub -noname @
5284 stub -noname @
5285 stub -noname @
5286 stub -noname @
5287 stub -noname @
5288 stub -noname @
5289 stub -noname @
5290 stub -noname @
5291 stub -noname @
5292 stub -noname @
5293 stub -noname @
5294 stdcall -noname XLivePBufferGetByteArray(long long long long)
5295 stdcall -noname XLivePBufferSetByteArray(long long long long)
5296 stub -noname @
5297 stdcall -noname XLIVE_5297()
5298 stub -noname @
5299 stub -noname @
5300 stub -noname @
5303 stub -noname @
5304 stub -noname @
5305 stub -noname @
5306 stub -noname @
5307 stub -noname @
5308 stub -noname @
5309 stub -noname @
5310 stdcall -noname XLIVE_5310()
5311 stdcall -noname XOnlineCleanup()
5312 stdcall -noname XFriendsCreateEnumerator(long long long long long)
5313 stdcall -noname XLIVE_5313(long)
5314 stub -noname @
5315 stub -noname @
5316 stub -noname @
5317 stub -noname @
5318 stub -noname @
5319 stub -noname @
5320 stub -noname @
5321 stub -noname @
5322 stub -noname @
5323 stub -noname @
5324 stub -noname @
5325 stub -noname @
5326 stub -noname @
5327 stub -noname @
5328 stub -noname @
5329 stub -noname @
5330 stub -noname @
5331 stdcall -noname XUserReadProfileSettings(long long long long long long long)
5332 stub -noname @
5333 stub -noname @
5334 stub -noname @
5335 stub -noname @
5336 stub -noname @
5337 stub -noname @
5338 stub -noname @
5339 stub -noname @
5340 stub -noname @
5341 stub -noname @
5342 stub -noname @
5343 stub -noname @
5344 stub -noname @
5345 stub -noname @
5346 stub -noname @
5347 stub -noname @
5348 stub -noname @
5349 stub -noname @
5350 stdcall -noname XLiveContentCreateAccessHandle(long long long long long long)
5351 stub -noname @
5352 stub -noname @
5354 stub -noname @
5355 stdcall -noname XLIVE_5355(long long long long)
5356 stdcall -noname XLIVE_5356(long long long long)
5357 stub -noname @
5358 stub -noname @
5359 stub -noname @
5360 stdcall -noname XLiveContentCreateEnumerator(long long long long)
5361 stub -noname @
5365 stub -noname @
