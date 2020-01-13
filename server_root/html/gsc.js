// Select the given ID and copies it:
const selectCopyId = (id) => {
  const elt = document.getElementById(id);
  elt.select();
  document.execCommand('copy')
};
