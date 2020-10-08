'use strict';
var exports;
var module = {
    get exports() {
        return require('boo!');
    }
};
var require = function (_) {
    throw new Error('Loader not ready');
};
var prepareLoader = function (moduleMap) {
    exports = {};
    module = { exports: exports };
    require = function (moduleName) {
        if (moduleName in moduleMap) {
            return moduleMap[moduleName];
        }
        else {
            throw new Error("Unknown module: " + moduleName);
        }
    };
    return exports;
};
