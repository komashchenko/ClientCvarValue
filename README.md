# ClientCvarValue

A MetaMod plugin that provides an API for querying CS2 client cvars, language, and operating system.

## Installation

1. Install [Metamod:Source](https://www.sourcemm.net/downloads.php?branch=dev).
2. Download a package for your platform from [Releases](https://github.com/komashchenko/ClientCvarValue/releases).
3. Extract the package into `game/csgo/` and restart the server.

## Usage in other MetaMod plugins

- Include the [public header](public/iclientcvarvalue.h).
- Obtain the interface in `AllPluginsLoaded`:

```cpp
IClientCvarValue* pClientCvarValue = static_cast<IClientCvarValue*>(g_SMAPI->MetaFactory(CLIENTCVARVALUE_INTERFACE, nullptr, nullptr));
```

- `QueryCvarValue` requests a client cvar and delivers its result through a callback.
- `GetClientLanguage` and `GetClientOS` return cached values, or `nullptr` if unavailable.
