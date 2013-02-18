1 stdcall -noname XWSAStartup(long long)
2 stdcall -noname XWSACleanup()
3 stdcall -noname XSocketCreate(long long long long)
4 stdcall -noname XSocketClose (long)
5 stdcall -noname XSocketShutdown (long long)
6 stdcall -noname XSocketIOCTLSocket(long long long)
7 stdcall -noname XSocketSetSockOpt(long long long long long)
8 stdcall -noname XSocketGetSockOpt(long long long long long)
9 stdcall -noname XSocketGetSockName (long long long)
10 stdcall -noname XSocketGetPeerName (long long long)
11 stdcall -noname XSocketBind (long long long)
12 stdcall -noname XSocketConnect (long long long)
13 stdcall -noname XSocketListen (long long)
14 stdcall -noname XSocketAccept (long long long)
15 stdcall -noname XSocketSelect (long long long long long)
16 stdcall -noname XWSAGetOverlappedResult (long long long long long)
17 stdcall -noname XWSACancelOverlappedIO (long)
18 stdcall -noname XSocketRecv (long long long long)
19 stdcall -noname XWSARecv (long long long long long long long)
20 stdcall -noname XSocketRecvFrom (long long long long long long)
21 stdcall -noname XWSARecvFrom (long long long long long long long long long)
22 stdcall -noname XSocketSend (long long long long)
23 stdcall -noname XWSASend (long long long long long long long)
24 stdcall -noname XSocketSendTo (long long long long long long)
25 stdcall -noname XWSASendTo (long long long long long long long long long)
26 stdcall -noname XSocketInet_Addr (long)
27 stdcall -noname XWSAGetLastError()
28 stdcall -noname XWSASetLastError(long)
29 stdcall -noname XWSACreateEvent()
30 stdcall -noname XWSACloseEvent(long)
31 stdcall -noname XWSASetEvent (long)
32 stdcall -noname XWSAResetEvent (long)
33 stdcall -noname XWSAWaitForMultipleEvents(long long long long long)
34 stdcall -noname __XWSAFDIsSet (long long)
35 stdcall -noname XWSAEventSelect (long long long)
37 stdcall -noname XSocketNTOHS(long)
38 stdcall -noname NetDll_ntohs(long)
39 stdcall -noname XSocketNTOHL(long)
40 stdcall -noname NetDll_htons(long)
51 stdcall -noname XNetStartup(long)
52 stdcall -noname XNetCleanup()
53 stdcall -noname XNetRandom (long long)
54 stdcall -noname XNetCreateKey (long long)
55 stdcall -noname XNetRegisterKey (long long)
56 stdcall -noname XNetUnregisterKey (long)
57 stdcall -noname XNetXnAddrToInAddr (long long long)
58 stdcall -noname XNetServerToInAddr (long long long)
60 stdcall -noname XNetInAddrToXnAddr (long long long)
62 stdcall -noname XNetInAddrToString (long long long)
63 stdcall -noname XNetUnregisterInAddr (long)
64 stdcall -noname XNetXnAddrToMachineId (long long)
65 stdcall -noname XNetConnect (long)
66 stdcall -noname XNetGetConnectStatus (long)
67 stdcall -noname XNetDnsLookup (long long long)
68 stdcall -noname XNetDnsRelease (long long long)
69 stdcall -noname XNetQosListen (long long long long long)
70 stdcall -noname XNetQosLookup (long long long long long long long long long long long long)
71 stdcall -noname NetDll_XNetQosServiceLookup (long long long)
72 stdcall -noname XNetQosRelease (long)
73 stdcall -noname XNetGetTitleXnAddr (long)
75 stdcall -noname XNetGetEthernetLinkStatus ()
76 stdcall -noname XNetGetBroadcastVersionStatus (long)
77 stdcall -noname XNetQosGetListenStats (long long)
78 stdcall -noname XNetGetOpt (long long long)
79 stdcall -noname XNetSetOpt (long long long)
81 stdcall -noname XNetReplaceKey (long long)
82 stdcall -noname XNetGetXnAddrPlatform (long long)
83 stdcall -noname XNetGetSystemLinkPort (long)
84 stdcall -noname XNetSetSystemLinkPort (long)
472 stdcall -noname XCustomSetAction (long long long)
473 stdcall -noname XCustomGetLastActionPress (long long long)
474 stdcall -noname XCustomSetDynamicActions (long long long long long)
476 stdcall -noname XCustomGetLastActionPressEx (long long long long long long)
477 stdcall -noname XCustomUnregisterDynamicActions ()
478 stdcall -noname XCustomRegisterDynamicActions ()
479 stdcall -noname XCustomGetCurrentGamercard (long long)
651 stdcall -noname XNotifyGetNext(long long long long)
652 stdcall -noname XNotifyPositionUI(long)
653 stdcall -noname XNotifyDelayUI(long)
1082 stdcall -noname XGetOverlappedExtendedError(long)
1083 stdcall -noname XGetOverlappedResult(long long long)
5000 stdcall -noname XLiveInitialize(long)
5001 stdcall -noname XliveInput(long)
5002 stdcall -noname XLiveRender()
5003 stdcall -noname XLiveUninitialize()
5005 stdcall -noname XLiveOnCreateDevice(long long)
5006 stdcall -noname XLIVE_5006()
5007 stdcall -noname XLiveOnResetDevice(long)
5008 stdcall -noname XHVCreateEngine(long long long)
5010 stub -noname @
5011 stub -noname @
5012 stub -noname @
5016 stdcall -noname XLivePBufferAllocate (long long)
5017 stdcall -noname XLivePBufferFree (long)
5018 stdcall -noname XLivePBufferGetByte (long long long)
5019 stdcall -noname XLivePBufferSetByte (long long long)
5020 stdcall -noname XLivePBufferGetDWORD (long long long)
5021 stdcall -noname XLivePBufferSetDWORD (long long long)
5022 stdcall -noname XLiveGetUpdateInformation (long)
5023 stub -noname @
5024 stdcall -noname XLiveUpdateSystem (long)
5025 stub -noname @
5026 stdcall -noname XLiveSetSponsorToken (long long)
5027 stub -noname @
5028 stub -noname @
5029 stub -noname @
5030 stdcall -noname XLivePreTranslateMessage(long)
5031 stdcall -noname XLiveSetDebugLevel(long long)
5032 stub -noname @
5034 stdcall -noname XLiveProtectData(long long long long long)
5035 stdcall -noname XLiveUnprotectData(long long long long long)
5036 stdcall -noname XLiveCreateProtectedDataContext (long long)
5037 stdcall -noname XLiveQueryProtectedDataInformation (long long)
5038 stdcall -noname XLiveCloseProtectedDataContext (long)
5039 stub -noname @
5206 stub -noname @
5208 stub -noname @
5209 stub -noname @
5210 stub -noname @
5212 stub -noname @
5214 stdcall -noname XShowPlayerReviewUI (long long long)
5215 stdcall -noname XShowGuideUI (long)
5216 stdcall -noname XShowKeyboardUI (long long long long long long long long)
5230 stub -noname @
5231 stub -noname @
5233 stub -noname @
5234 stub -noname @
5235 stub -noname @
5236 stdcall -noname XLIVE_5236(long long)
5237 stdcall -noname XLIVE_5237(long)
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
5260 stdcall -noname XShowSigninUI(long long)
5261 stdcall -noname XUserGetXUID(long long)
5262 stdcall -noname XUserGetSigninState(long)
5263 stdcall -noname XUserGetName(long long long)
5264 stdcall -noname XUserAreUsersFriends (long long long long long)
5265 stdcall -noname XLiveUserCheckPrivilege(long long long)
5266 stub -noname @
5267 stdcall -noname XUserGetSigninInfo(long long long)
5270 stdcall -noname XNotifyCreateListener(long long)
5271 stub -noname @
5273 stdcall -noname XUserReadGamerpictureByKey (long long long long long long)
5274 stdcall -noname XLIVE_5274(long long long long)
5275 stdcall -noname XShowFriendsUI (long)
5276 stdcall -noname XUserSetProperty(long long long long)
5277 stdcall -noname XUserSetContext(long long long)
5278 stdcall -noname XUserWriteAchievements(long long long)
5279 stub -noname @
5280 stdcall -noname XUserCreateAchievementEnumerator(long long long long long long long long long)
5281 stdcall -noname XUserReadStats(long long long long long long long long)
5282 stdcall -noname XLIVE_5282 (long long long long long long)
5284 stdcall -noname XUserCreateStatsEnumeratorByRank (long long long long long long long)
5285 stub -noname @
5286 stdcall -noname XUserCreateStatsEnumeratorByXuid (long long long long long long long long)
5287 stub -noname @
5288 stub -noname @
5289 stub -noname @
5290 stub -noname @
5291 stub -noname @
5292 stdcall -noname XUserSetContextEx (long long long long)
5293 stdcall -noname XUserSetPropertyEx(long long long long long)
5294 stdcall -noname XLivePBufferGetByteArray(long long long long)
5295 stdcall -noname XLivePBufferSetByteArray(long long long long)
5296 stub -noname @
5297 stdcall -noname XLiveInitializeEx(long long)
5298 stub -noname @
5299 stub -noname @
5300 stdcall -noname XSessionCreate(long long long long long long long long)
5303 stdcall -noname XStringVerify (long long long long long long long)
5304 stub -noname @
5305 stdcall -noname XStorageUploadFromMemory (long long long long long)
5306 stdcall -noname XStorageEnumerate (long long long long long long long)
5307 stub -noname @
5308 stub -noname @
5309 stub -noname @
5310 stdcall -noname XOnlineStartup()
5311 stdcall -noname XOnlineCleanup()
5312 stdcall -noname XFriendsCreateEnumerator (long long long long long)
5313 stdcall -noname XLIVE_5313(long)
5314 stdcall -noname XUserMuteListQuery (long long long long)
5315 stdcall -noname XInviteGetAcceptedInfo (long long)
5316 stdcall -noname XInviteSend (long long long long long)
5317 stdcall -noname XSessionWriteStats (long long long long long long)
5318 stdcall -noname XSessionStart (long long long)
5319 stdcall -noname XSessionSearchEx (long long long long long long long long long long long)
5320 stub -noname @
5321 stub -noname @
5322 stdcall -noname XSessionModify (long long long long long)
5323 stdcall -noname XSessionMigrateHost (long long long long)
5324 stdcall -noname XOnlineGetNatType()
5325 stdcall -noname XSessionLeaveLocal (long long long long)
5326 stdcall -noname XSessionJoinRemote(long long long long long)
5327 stdcall -noname XSessionJoinLocal (long long long long long)
5328 stdcall -noname XSessionGetDetails (long long long long)
5329 stdcall -noname XSessionFlushStats (long long)
5330 stdcall -noname XSessionDelete (long long)
5331 stdcall -noname XUserReadProfileSettings(long long long long long long long)
5332 stdcall -noname XSessionEnd (long long)
5333 stdcall -noname XSessionArbitrationRegister (long long long long long long long)
5334 stub -noname @
5335 stdcall -noname XTitleServerCreateEnumerator (long long long long)
5336 stdcall -noname XSessionLeaveRemote (long long long long)
5337 stdcall -noname XUserWriteProfileSettings(long long long long)
5338 stub -noname @
5339 stdcall -noname XUserReadProfileSettingsByXuid(long long long long long long long long long)
5340 stub -noname @
5341 stub -noname @
5342 stub -noname @
5343 stdcall -noname XLiveCalculateSkill (long long long long long)
5344 stdcall -noname XStorageBuildServerPath (long long long long long long long)
5345 stdcall -noname XStorageDownloadToMemory (long long long long long long long)
5346 stub -noname @
5347 stub -noname @
5348 stub -noname @
5349 stdcall -noname XLiveProtectedVerifyFile (long long long)
5350 stdcall -noname XLiveContentCreateAccessHandle(long long long long long long)
5351 stub -noname @
5352 stdcall -noname XLiveContentUninstall (long long long)
5354 stub -noname @
5355 stdcall -noname XLiveContentGetPath(long long long long)
5356 stdcall -noname XLIVE_5356(long long long long)
5357 stub -noname @
5358 stub -noname @
5359 stub -noname @
5360 stdcall -noname XLiveContentCreateEnumerator(long long long long)
5361 stdcall -noname XLiveContentRetrieveOffersByDate (long long long long long long)
5365 stdcall -noname XShowMarketplaceUI (long long long long)
5372 stdcall -noname XLIVE_5372(long long long long long long)
