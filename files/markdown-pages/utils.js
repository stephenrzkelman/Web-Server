function add_redirection_links() {
    let base_url = "http://fortnite-gamers.org/markdown-manager/";
    let create_message = document.createElement("h2");
    create_message.innerHTML = "Create a new Markdown file: ";
    contents.appendChild(create_message);
    let create_link = document.createElement("a");
    let create_url = base_url + "create.html";
    create_link.innerHTML = create_url;
    create_link.href = create_url;
    create_message.appendChild(create_link);

    let update_message = document.createElement("h2");
    update_message.innerHTML = "Update a Markdown file: ";
    contents.appendChild(update_message);
    let update_link = document.createElement("a");
    let update_url = base_url + "update.html";
    update_link.innerHTML = update_url;
    update_link.href = update_url;
    update_message.appendChild(update_link);

    let delete_message = document.createElement("h2");
    delete_message.innerHTML = "Delete a Markdown file: ";
    contents.appendChild(delete_message);
    let delete_link = document.createElement("a");
    let delete_url = base_url + "delete.html";
    delete_link.innerHTML = delete_url;
    delete_link.href = delete_url;
    delete_message.appendChild(delete_link);
}