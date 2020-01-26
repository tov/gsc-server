var exports

var module = {
    get exports() {
        return require('boo!')
    }
}

var require: (_: string) => any = (_) => {
    throw new Error('Loader not ready')
}

var resetLoader = (moduleMap: any) => {
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
