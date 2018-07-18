## Example

[SOURCE*click*](../blob/master/include/main.cpp)
![alt text](https://cdn.discordapp.com/attachments/466954372660723714/469097710356660245/unknown.png "example illustration")

## Usage

#### Include: 
```
git submodule add https://gitlab.mycrutches.ru/Bulb4/renderer.git
```

```java
#include "../include/renderer.h"
```

***
#### Initializing:
```cpp
cRender* pRender = new cRender(g_pd3dDevice);//g_pd3dDevice is our IDirect3DDevice9

ID3DXFont* font1 = nullptr;
pRender->AddFont(&font1, "Consolas", 48, false);

pRender->SetFramerateUpdateRate(400U);
```
***

#### In your drawing function(EndScene, Present, etc.):
```cpp
pRender->BeginDraw();
 
//drawing stuff
 
pRender->EndDraw();
```
***
#### In your Reset:
```cpp
pRender->OnLostDevice();

if (g_pd3dDevice->Reset(&g_d3dpp) >= 0)
    pRender->OnResetDevice();
```
***
#### Notes:
* Use `pRender->GetFramerate()` for fps monitoring, you can change it's update rate by `pRender->SetFramerateUpdateRate(400U);//ms`
* Use `pRender->PushRenderState(...)` if you need to use some render states while drawing, but you need to return it's previous value after.
 
***
## Contacts:
* Discord: `Bulb4#2901`
* Steam: [`Bulb4<3`](https://steamcommunity.com/id/bulb4_/)
* e-mail: `bulb4main@gmail.com`

***
