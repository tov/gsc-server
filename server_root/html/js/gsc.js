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
    exports.FileViewer = exports.selectCopyId = void 0;
    var $ = require("jquery");
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
    var FileViewer = /** @class */ (function () {
        function FileViewer(element) {
            var viewer = $(element);
            this._select = viewer.find('.file-viewer-selector');
            this._scrollArea = viewer.find('.file-viewer-area');
            this._lines = this._scrollArea
                .find('tr.numbered td.code-line')
                .get();
            this._files = this._scrollArea
                .find('div.single-file-viewer')
                .get().map($);
            this._select.on('change', this.showSelectedFile.bind(this));
            this._scrollArea.on('scroll', this.selectShownFile.bind(this));
            this.setupLineLinks();
            viewer.data(VIEWER_KEY, this);
        }
        FileViewer.forId = function (id) {
            var _a;
            var viewer = document.getElementById(id);
            return (_a = $.data(viewer, VIEWER_KEY)) !== null && _a !== void 0 ? _a : new FileViewer(viewer);
        };
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
            var value = $(lines[index - 1]);
            if (value.length) {
                lines[index - 1] = value;
            }
            return value;
        };
        FileViewer.prototype.setupLineLinks = function () {
            var lineCount = this._lines.length;
            $('.line-link')
                .each(function (_, link) {
                var number = extractNumber(link.innerText);
                $.data(link, LINE_NUMBER_KEY, number);
                if (number > lineCount) {
                    link.className = 'broken-line-link';
                }
            })
                .on('click', this.handleLineLink.bind(this));
        };
        FileViewer.prototype.handleLineLink = function (evt) {
            this.showLine($.data(evt.target, LINE_NUMBER_KEY));
        };
        FileViewer.prototype.showSelectedFile = function () {
            var choice = this._select.val();
            this.showFile(Number(choice));
        };
        FileViewer.prototype.selectShownFile = function () {
            var _a;
            this._select.val((_a = this.currentShownFile()) !== null && _a !== void 0 ? _a : 0);
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
            var area = this._scrollArea;
            var goal = target.offset().top - margin * area.height();
            var delta = goal - area.offset().top;
            area.scrollTop(area.scrollTop() + delta);
            target.effect('highlight', { color: 'purple' }, 500);
        };
        FileViewer.prototype.error = function () {
            this._scrollArea.effect('shake', { distance: 10, times: 2 }, 400);
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
        return elt.position().top + ((_a = elt.height()) !== null && _a !== void 0 ? _a : 0);
    }
});
//# sourceMappingURL=gsc.js.map