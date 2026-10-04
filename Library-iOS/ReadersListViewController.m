//
//  ReadersListViewController.m
//
#import "ReadersListViewController.h"
#import "LBLibraryStore.h"
#import "LBReader.h"

@interface ReadersListViewController () <UITableViewDataSource, UITableViewDelegate>
@property (nonatomic, strong) UITableView *tableView;
@property (nonatomic, strong) NSArray<LBReader *> *readers;
@end

@implementation ReadersListViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"读者";
    self.view.backgroundColor = [UIColor systemBackgroundColor];

    self.tableView = [[UITableView alloc] initWithFrame:CGRectZero
                                                  style:UITableViewStylePlain];
    self.tableView.dataSource = self;
    self.tableView.delegate = self;
    [self.tableView registerClass:[UITableViewCell class]
           forCellReuseIdentifier:@"ReaderCell"];
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
                                                      action:@selector(addReaderTapped)];

    [self reloadData];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    [self reloadData];
}

- (void)reloadData {
    self.readers = [[LBLibraryStore sharedStore] readers];
    [self.tableView reloadData];
}

- (void)addReaderTapped {
    UIAlertController *alert = [UIAlertController
        alertControllerWithTitle:@"新增读者" message:nil
                  preferredStyle:UIAlertControllerStyleAlert];
    [alert addTextFieldWithConfigurationHandler:^(UITextField *tf) {
        tf.placeholder = @"姓名";
    }];
    [alert addTextFieldWithConfigurationHandler:^(UITextField *tf) {
        tf.placeholder = @"联系电话（可空）";
        tf.keyboardType = UIKeyboardTypePhonePad;
    }];
    UIAlertAction *ok = [UIAlertAction actionWithTitle:@"保存"
                                                 style:UIAlertActionStyleDefault
                                               handler:^(UIAlertAction *action) {
        NSString *err = nil;
        LBReader *r = [[LBLibraryStore sharedStore] addReaderName:alert.textFields[0].text
                                                            phone:alert.textFields[1].text
                                                            error:&err];
        if (!r) {
            UIAlertController *tip = [UIAlertController alertControllerWithTitle:@"提示"
                                                                         message:err ?: @"添加失败"
                                                                  preferredStyle:UIAlertControllerStyleAlert];
            [tip addAction:[UIAlertAction actionWithTitle:@"好"
                                                    style:UIAlertActionStyleDefault handler:nil]];
            [self presentViewController:tip animated:YES completion:nil];
        }
        [self reloadData];
    }];
    UIAlertAction *cancel = [UIAlertAction actionWithTitle:@"取消"
                                                    style:UIAlertActionStyleCancel handler:nil];
    [alert addAction:ok];
    [alert addAction:cancel];
    [self presentViewController:alert animated:YES completion:nil];
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.readers.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"ReaderCell"
                                                            forIndexPath:indexPath];
    LBReader *r = self.readers[indexPath.row];
    cell.textLabel.text = r.name;
    cell.detailTextLabel.text = r.phone.length ? r.phone : @"（无电话）";
    return cell;
}

- (UISwipeActionsConfiguration *)tableView:(UITableView *)tableView
         trailingSwipeActionsConfigurationForRowAtIndexPath:(NSIndexPath *)indexPath {
    UIContextualAction *del = [UIContextualAction
        contextualActionWithStyle:UIContextualActionStyleDestructive
                            title:@"删除"
                          handler:^(UIContextualAction *action, __kindof UIView *sourceView,
                                    void (^completion)(BOOL)) {
        LBReader *r = self.readers[indexPath.row];
        NSString *err = nil;
        if ([[LBLibraryStore sharedStore] removeReader:r.readerId error:&err]) {
            [self reloadData];
            completion(YES);
        } else {
            completion(NO);
            UIAlertController *tip = [UIAlertController alertControllerWithTitle:@"提示"
                                                                         message:err ?: @"删除失败"
                                                                  preferredStyle:UIAlertControllerStyleAlert];
            [tip addAction:[UIAlertAction actionWithTitle:@"好" style:UIAlertActionStyleDefault handler:nil]];
            [self presentViewController:tip animated:YES completion:nil];
        }
    }];
    return [UISwipeActionsConfiguration configurationWithActions:@[del]];
}

@end
