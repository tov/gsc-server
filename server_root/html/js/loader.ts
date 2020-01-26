'use strict';

let exports: any

let module = {
    get exports() {
        return require('boo!')
    }
}

let require = (_: string): any => {
    throw new Error('Loader not ready')
}

const resetLoader = (moduleMap: any) => {
    exports = {}

    module = {exports: exports}

    require = (moduleName: string): any => {
        if (moduleName in moduleMap) {
            return moduleMap[moduleName]
        } else {
            throw new Error(`Unknown module: ${moduleName}`)
        }
    }
}
