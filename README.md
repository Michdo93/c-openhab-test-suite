# c-openhab-test-suite

A C testing library for validating openHAB installations.
Mirrors the Python [openhab-test-suite](https://github.com/Michdo93/openhab-test-suite)
with the same logic, adapted to C11 naming: `openhab_<tester>_<method>(client, ...)`.

Powered by [c-openhab-rest-client](https://github.com/Michdo93/c-openhab-rest-client).

## Function naming

| Python | C |
|---|---|
| `ItemTester.doesItemExist(name)` | `openhab_item_tester_does_item_exist(c, name)` |
| `ThingTester.isThingOnline(uid)` | `openhab_thing_tester_is_thing_online(c, uid)` |
| `RuleTester.runRule(uid)` | `openhab_rule_tester_run_rule(c, uid, NULL)` |
| `ChannelTester.hasOrphanedLinks()` | `openhab_channel_tester_has_orphaned_links(c)` |
| `PersistenceTester.isItemPersisted(svc, item)` | `openhab_persistence_tester_is_item_persisted(c, svc, item)` |
| `SitemapTester.doesSitemapExist(name)` | `openhab_sitemap_tester_does_sitemap_exist(c, name)` |

## Adding to CMake

```cmake
add_subdirectory(extern/c-openhab-test-suite)
target_link_libraries(myapp PRIVATE OpenHABTestSuiteC::Static)
```

## Build & Test

```bash
git clone --recurse-submodules https://github.com/Michdo93/c-openhab-test-suite.git
cd c-openhab-test-suite && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build .
OPENHAB_URL=http://127.0.0.1:8080 OPENHAB_USER=openhab OPENHAB_PASS=habopen ./openhab_suite_test
```

## License

MIT
