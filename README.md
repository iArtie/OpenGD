<div align="center">

![issues](https://img.shields.io/github/issues/Open-GD/OpenGD?style=for-the-badge&color=blue)
![forks](https://img.shields.io/github/forks/Open-GD/OpenGD?style=for-the-badge)
![stars](https://img.shields.io/github/stars/Open-GD/OpenGD?style=for-the-badge&color=blue)
![LICENSE](https://img.shields.io/github/license/Open-GD/OpenGD?style=for-the-badge&color=blue)
<a href="https://discord.gg/gcbuuR4JWg">
<img src="https://dcbadge.vercel.app/api/server/gcbuuR4JWg">
</a>
</div>

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/Open-GD/OpenGD/releases/latest">
    <img src="https://user-images.githubusercontent.com/54410739/226145157-61edd6d9-eec4-479c-83b6-3f0c32e278c3.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">OpenGD</h3>

  <p align="center">
    Implementación de código abierto de Geometry Dash
    <br />   
  </p>
  
![](https://img.shields.io/badge/platforms-windows%20%7C%20linux%20%7C%20mac%20%7C%20android%20%7C%20ios-blue)
    <p align="center">
    <a href="https://github.com/Open-GD/OpenGD/issues">Reportar bugs</a>
    ·
    <a href="https://github.com/Open-GD/OpenGD/releases/latest">Latest Release</a>
 · 
 <a href="https://github.com/Open-GD/OpenGD/issues">Request Feature</a>
  </p>
</div>


<!-- # UNMAINTAINED
new (unfinished) projects are [gdrender](https://github.com/maxnut/gdrender) and [gdclone](https://github.com/opstic/gdclone)
ABOUT THE PROJECT -->
## Acerca del proyecto

<!-- ![Stereo Madness running in OpenGD](https://cdn.discordapp.com/attachments/847950548921614366/1086798200146497647/6046uyhlekoa1.png "OpenGD") -->

OpenGD es una implementación de código abierto del popular juego Geometry Dash. Nuestro objetivo principal es recrear la jugabilidad al 100 %, al tiempo que mejoramos el rendimiento mediante nuevas funciones del motor y mejoras en C++. También tenemos previsto implementar el multihilo en el futuro.

## Status 

Actualmente estamos reescribiendo la mecánica del juego desde cero, **por lo que los niveles no están disponibles por el momento**.

### Creado con

OpenGD se basa en [axmol](https://github.com/axmolengine/axmol), una bifurcación de cocos2dx 4.0 que incorpora numerosas funciones nuevas y mejoras con respecto al cocos2dx original. El Geometry Dash original también está creado con cocos2dx, pero con una versión mucho más antigua, de 2014.

## Instrucciones de montaje

Requisitos:
- PowerShell
- CMake
- Compilador C++20 (MSVC, clang o gcc)


<details>

  <summary>Windows</summary>

### Inicio rápido

Descarga axmol, ejecuta setup.ps1 y reinicia cmd o Powershell para que se actualicen las variables de la línea de comandos
### [Recomendamos utilizar Axmol 2.2.1. {https://github.com/axmolengine/axmol/releases?page=2}]
```
./setup.ps1
```

En la carpeta OpenGD, compila con CMake como de costumbre
```
cmake -B build_x64
cmake --build build_x64 --config RelWithDebInfo
```

> **Advertencia**
> Es posible que VS 2019 no funcione en Windows; se recomienda utilizar VS 2022

<img width="972" height="228" alt="image" src="https://github.com/user-attachments/assets/f827bd63-5125-4aff-ae62-38912e00f2e4" />

### NOTA: Si te sale el mismo error al compilar el proyecto, lo unico que tienes que hacer es ejecutar el siguiente comando en CMD o en Powershell, reemplaza la parte final del comando a la carpeta real de los assets de GD si no quieres agregar la carpeta Content, tendrás que tener abierto GD al compilar:

```
New-Item -ItemType Junction -Path "C:\Users\{nombre de usuario}\{directorio en donde está almacenado el proyecto}\OpenGD\Content" -Target "{Directorio de la carpeta real}"
```

### Configuración recomendada: VSCode

Requisitos:
  - Ninja
  - clang (llvm)
  - Extensión cmake-tools
  - Extensión c/c++

Recomendado: [sccache](https://github.com/mozilla/sccache) (recompilaciones más rápidas)

¡Asegúrate de que ninja y clang estén en la ruta de acceso!

En cmake-tools, selecciona la configuración «Ninja default» o «Ninja sccache» y, a continuación, compila con cmake-tools o con «cmake --build build».

La configuración de VSCode ofrece compatibilidad con IntelliSense y el depurador (requiere VS2022).

</details>

<details>

<summary>Otras plataformas</summary>
  
Echa un vistazo a axmol [Dev setup](https://github.com/axmolengine/axmol/blob/dev/docs/DevSetup.md)

</details>

Para poder ejecutar el juego, necesitarás los recursos de la versión 2.2/2.1 de Geometry Dash.

<!-- LICENSE -->
## License

Distributed under the GPL v3 License . See `LICENSE` for more information.

<!-- ACKNOWLEDGMENTS -->
## Creditos

* [axmol](https://github.com/axmolengine/axmol) a fork of cocos2d-x-4.0
* [GD 1.0 decomps](https://github.com/Wyliemaster/Geometry-Dash-1.0) by Wylie
* [GD Physics decomps](https://github.com/camila314/gdp) by Camila
* [GD 2.1 decomps](https://github.com/matcool/gd-decomps) by mat
* [hps](https://github.com/jl2922/hps) high performance C++11 serialization library
* [gdclone](https://github.com/opstic/gdclone) another gd reconstruction project

### Colaboradores
Este proyecto es posible gracias a todas las personas que han colaborado en él:

<a href="https://github.com/Open-GD/OpenGD/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=Open-GD/OpenGD" />
</a>
