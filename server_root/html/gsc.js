// Select the given ID and copies it:
const selectCopyId = (id) => {
  const elt = document.getElementById(id);
  elt.select();
  document.execCommand('copy')
};

const scrollToId = (viewerId, targetId) => {
  const viewer = $('#' + viewerId);
  const target = $('#' + targetId);
  if (target && target.position() && target.position().top) {
    const top = target.position().top;
    const goal = top > 100 ? top - 100 : 0;
    viewer.scrollTop(0);
    viewer.scrollTop(goal)
  }
};
