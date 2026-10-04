//
//  BooksListViewController.m
//
#import "BooksListViewController.h"
#import "LBLibraryStore.h"
#import "LBBook.h"

@interface BooksListViewController () <UITableViewDataSource, UITableViewDelegate,
                                       UISearchBarDelegate>
@property (nonatomic, strong) UITableView *tableView;
@property (nonatomic, strong) UISearchBar *searchBar;
@property (nonatomic, strong) NSArray<LBBook *> *displayBooks;
@end

@implementation BooksListViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"图书";
    self.view.backgroundColor = [UIColor systemBackgroundColor];

    self.searchBar = [[UISearchBar alloc] initWithFrame:CGRectZero];
    self.searchBar.placeholder = @"按书名 / 作者 / ISBN 搜索";
    self.searchBar.delegate = self;

    self.tableView = [[UITableView alloc] initWithFrame:CGRectZero
                                                  style:UITableViewStylePlain];
    self.tableView.dataSource = self;
    self.tableView.delegate = self;
    self.tableView.tableHeaderView = self.searchBar;
    [self.tableView registerClass:[UITableViewCell class]
           forCellReuseIdentifier:@"BookCell"];
    self.tableView.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:self.tableView];

    [NSLayoutConstraint activateConstraints:@[
        [self.tableView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [self.tableView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor],
        [self.tableView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [self.tableView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
    ]];

    self.navigationItem.rightBarButtonItem =
        [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd
                                                      target:self
                                                      action:@selector(addBookTapped)];

    [self reloadData];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    [self reloadData];
}

- (void)reloadData {
    NSString *kw = self.searchBar.text ?: @"";
    self.displayBooks = [[LBLibraryStore sharedStore] searchBooks:kw];
    [self.tableView reloadData];
}

#pragma mark - 新增/编辑

- (void)addBookTapped {
    [self showBookFormWithBook:nil];
}

- (void)showBookFormWithBook:(nullable LBBook *)book {
    UIAlertController *alert = [UIAlertController
        alertControllerWithTitle:book ? @"修改图书" : @"新增图书"
                         message:nil
                  preferredStyle:UIAlertControllerStyleAlert];
    [alert addTextFieldWithConfigurationHandler:^(UITextField *tf) {
        tf.placeholder = @"书名";
        tf.text = book.title;
    }];
    [alert addTextFieldWithConfigurationHandler:^(UITextField *tf) {
        tf.placeholder = @"作者";
        tf.text = book.author;
    }];
    [alert addTextFieldWithConfigurationHandler:^(UITextField *tf) {
        tf.placeholder = @"ISBN（可空）";
        tf.text = book.isbn;
    }];
    [alert addTextFieldWithConfigurationHandler:^(UITextField *tf) {
        tf.placeholder = @"馆藏册数";
        tf.keyboardType = UIKeyboardTypeNumberPad;
        tf.text = book ? [NSString stringWithFormat:@"%ld", (long)book.totalCopies] : @"1";
    }];

    UIAlertAction *ok = [UIAlertAction actionWithTitle:@"保存"
                                                 style:UIAlertActionStyleDefault
                                               handler:^(UIAlertAction *action) {
        UITextField *t1 = alert.textFields[0];
        UITextField *t2 = alert.textFields[1];
        UITextField *t3 = alert.textFields[2];
        UITextField *t4 = alert.textFields[3];
        NSInteger copies = 1;
        if (t4.text.length > 0) copies = t4.text.integerValue;
        NSString *err = nil;
        BOOL ok = NO;
        if (book) {
            ok = [[LBLibraryStore sharedStore] updateBook:book.bookId
                                                    title:t1.text
                                                   author:t2.text
                                                     isbn:t3.text
                                                   copies:copies
                                                    error:&err];
        } else {
            LBBook *b = [[LBLibraryStore sharedStore] addBookTitle:t1.text
                                                            author:t2.text
                                                              isbn:t3.text
                                                            copies:copies
                                                             error:&err];
            ok = (b != nil);
        }
        if (!ok) [self showMessage:err ?: @"操作失败"];
        [self reloadData];
    }];
    UIAlertAction *cancel = [UIAlertAction actionWithTitle:@"取消"
                                                    style:UIAlertActionStyleCancel
                                                  handler:nil];
    [alert addAction:ok];
    [alert addAction:cancel];
    [self presentViewController:alert animated:YES completion:nil];
}

- (void)showMessage:(NSString *)message {
    UIAlertController *alert = [UIAlertController
        alertControllerWithTitle:@"提示" message:message
                  preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"好"
                                              style:UIAlertActionStyleDefault
                                            handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.displayBooks.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"BookCell"
                                                            forIndexPath:indexPath];
    LBBook *b = self.displayBooks[indexPath.row];
    NSString *sub = [NSString stringWithFormat:@"%@ ｜ 共%ld册 可借%ld册%@",
                     b.author.length ? b.author : @"佚名",
                     (long)b.totalCopies, (long)b.availableCopies,
                     b.isbn.length ? [NSString stringWithFormat:@" ｜ %@", b.isbn] : @""];
    cell.textLabel.text = b.title;
    cell.detailTextLabel.text = sub;
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    return YES;
}

- (UISwipeActionsConfiguration *)tableView:(UITableView *)tableView
         trailingSwipeActionsConfigurationForRowAtIndexPath:(NSIndexPath *)indexPath {
    UIContextualAction *del = [UIContextualAction
        contextualActionWithStyle:UIContextualActionStyleDestructive
                            title:@"删除"
                          handler:^(UIContextualAction *action, __kindof UIView *sourceView,
                                    void (^completion)(BOOL)) {
        LBBook *b = self.displayBooks[indexPath.row];
        NSString *err = nil;
        if ([[LBLibraryStore sharedStore] removeBook:b.bookId error:&err]) {
            [self reloadData];
            completion(YES);
        } else {
            completion(NO);
            [self showMessage:err ?: @"删除失败"];
        }
    }];
    return [UISwipeActionsConfiguration configurationWithActions:@[del]];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    LBBook *b = self.displayBooks[indexPath.row];
    [self showBookFormWithBook:b];
}

#pragma mark - UISearchBarDelegate

- (void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)searchText {
    [self reloadData];
}

- (void)searchBarSearchButtonClicked:(UISearchBar *)searchBar {
    [searchBar resignFirstResponder];
}

@end
