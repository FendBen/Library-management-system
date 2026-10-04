//
//  AppDelegate.m
//
#import "AppDelegate.h"
#import "LBLibraryStore.h"
#import "BooksListViewController.h"
#import "ReadersListViewController.h"
#import "BorrowViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {

    [[LBLibraryStore sharedStore] load];

    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];

    BooksListViewController *books = [[BooksListViewController alloc] init];
    UINavigationController *navBooks = [[UINavigationController alloc] initWithRootViewController:books];
    navBooks.tabBarItem = [[UITabBarItem alloc] initWithTitle:@"图书"
                                                        image:[UIImage systemImageNamed:@"book"]
                                                  selectedImage:nil];

    ReadersListViewController *readers = [[ReadersListViewController alloc] init];
    UINavigationController *navReaders = [[UINavigationController alloc] initWithRootViewController:readers];
    navReaders.tabBarItem = [[UITabBarItem alloc] initWithTitle:@"读者"
                                                          image:[UIImage systemImageNamed:@"person.2"]
                                                    selectedImage:nil];

    BorrowViewController *borrow = [[BorrowViewController alloc] init];
    UINavigationController *navBorrow = [[UINavigationController alloc] initWithRootViewController:borrow];
    navBorrow.tabBarItem = [[UITabBarItem alloc] initWithTitle:@"借阅"
                                                         image:[UIImage systemImageNamed:@"arrow.left.arrow.right"]
                                                   selectedImage:nil];

    UITabBarController *tab = [[UITabBarController alloc] init];
    tab.viewControllers = @[navBooks, navReaders, navBorrow];

    self.window.rootViewController = tab;
    [self.window makeKeyAndVisible];
    return YES;
}

@end
