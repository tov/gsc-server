var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
(function (factory) {
    if (typeof module === "object" && typeof module.exports === "object") {
        var v = factory(require, exports);
        if (v !== undefined) module.exports = v;
    }
    else if (typeof define === "function" && define.amd) {
        define(["require", "exports", "jquery"], factory);
    }
})(function (require, exports) {
    'use strict';
    Object.defineProperty(exports, "__esModule", { value: true });
    var jquery_1 = __importDefault(require("jquery"));
    var VIEWER_KEY = 'gsc file viewer';
    var LINE_NUMBER_KEY = 'gsc line number';
    var DIGITS_RE = /[0-9]+/;
    function selectCopyId(inputId) {
        var elt = document.getElementById(inputId);
        if (elt instanceof HTMLInputElement) {
            elt.select();
            document.execCommand('copy');
        }
        else {
            console.log("selectCopyId: could not select " + inputId);
        }
    }
    exports.selectCopyId = selectCopyId;
    function fileViewerFor(id) {
        var _a;
        var viewer = document.getElementById(id);
        return _a = jquery_1.default.data(viewer, VIEWER_KEY), (_a !== null && _a !== void 0 ? _a : new FileViewer(viewer));
    }
    exports.fileViewerFor = fileViewerFor;
    var FileViewer = /** @class */ (function () {
        function FileViewer(element) {
            var viewer = jquery_1.default(element);
            var selector = viewer.find('.file-viewer-selector');
            var area = viewer.find('.file-viewer-area');
            this._viewer = viewer;
            this._selector = selector;
            this._area = area;
            this._lines = area.find('td.code-line')
                .get();
            this._files = area.find('div.single-file-viewer')
                .get().map(jquery_1.default);
            selector.on('change', this.showSelectedFile.bind(this));
            area.on('scroll', this.selectShownFile.bind(this));
            this.setupLineLinks();
            viewer.data(VIEWER_KEY, this);
        }
        FileViewer.prototype.showLine = function (lineNo) {
            this.showElement(this.line(lineNo), 0.3);
        };
        FileViewer.prototype.showFile = function (fileNo) {
            this.showElement(this.file(fileNo), 0);
        };
        FileViewer.prototype.file = function (index) {
            return this._files[index];
        };
        FileViewer.prototype.line = function (index) {
            var lines = this._lines;
            var value = jquery_1.default(lines[index - 1]);
            if (value.length) {
                lines[index - 1] = value;
            }
            return value;
        };
        FileViewer.prototype.setupLineLinks = function () {
            var lineCount = this._lines.length;
            jquery_1.default('.line-link')
                .each(function (_, link) {
                var number = extractNumber(link.innerText);
                jquery_1.default.data(link, LINE_NUMBER_KEY, number);
                if (number > lineCount) {
                    link.className = 'broken-line-link';
                }
            })
                .on('click', this.handleLineLink.bind(this));
        };
        FileViewer.prototype.handleLineLink = function (evt) {
            this.showLine(jquery_1.default.data(evt.target, LINE_NUMBER_KEY));
        };
        FileViewer.prototype.showSelectedFile = function () {
            var choice = this._selector.val();
            this.showFile(Number(choice));
        };
        FileViewer.prototype.selectShownFile = function () {
            var _a;
            this._selector.val((_a = this.currentShownFile(), (_a !== null && _a !== void 0 ? _a : 0)));
        };
        FileViewer.prototype.currentShownFile = function () {
            for (var _i = 0, _a = this._files; _i < _a.length; _i++) {
                var file = _a[_i];
                if (bottom(file) >= 0) {
                    return file.parent().index();
                }
            }
        };
        FileViewer.prototype.showElement = function (target, margin) {
            if (!target.length) {
                this.error();
                return;
            }
            var area = this._area;
            var goal = target.offset().top - margin * area.height();
            var delta = goal - area.offset().top;
            area.scrollTop(area.scrollTop() + delta);
            target.effect('highlight', { color: 'purple' }, 500);
        };
        FileViewer.prototype.error = function () {
            this._area.effect('shake', { distance: 10, times: 2 }, 400);
        };
        return FileViewer;
    }());
    exports.FileViewer = FileViewer;
    function extractNumber(str) {
        var execResult = DIGITS_RE.exec(str);
        return execResult ? parseInt(execResult[0]) : 0;
    }
    function bottom(elt) {
        var _a;
        return elt.position().top + (_a = elt.height(), (_a !== null && _a !== void 0 ? _a : 0));
    }
});
//# sourceMappingURL=gsc.js.map