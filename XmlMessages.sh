function get_files
{
    echo kile.xml
}

function po_for_file
{
    case "$1" in
       kile.xml)
           echo kile_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       kile.xml)
           echo comment
       ;;
    esac
}
